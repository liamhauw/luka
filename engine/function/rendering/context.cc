// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/context.h"

#include "core/log.h"
#include "function/gpu/gpu.h"

namespace luka {

namespace rd {

Context::Context(std::shared_ptr<Asset> asset, std::shared_ptr<Window> window,
                 std::shared_ptr<Gpu> gpu,
                 std::shared_ptr<SceneGraph> scene_graph)
    : asset_{asset}, window_{window}, gpu_{gpu}, scene_graph_{scene_graph} {
  CreateSwapchain();
  CreateFrames();
  CreateAcquiredSemphores();
  CreateViewportAndScissor();
  CreatePasses();
}

Context::~Context() { gpu_->WaitIdle(); }

void Context::Resize() {}

void Context::Draw() {
  const vk::raii::CommandBuffer& command_buffer{Begin()};

  TarversePasses(command_buffer);

  End(command_buffer);
}

const vk::raii::CommandBuffer& Context::Begin() {
  vk::Result result;
  std::tie(result, active_frame_index_) = swapchain_.acquireNextImage(
      UINT64_MAX, *(acquired_semaphores_[acquired_semaphore_index]), nullptr);
  if (result != vk::Result::eSuccess) {
    THROW("Fail to acqurie next image.");
  }

  auto& cur_frame{frames_[active_frame_index_]};
  const vk::raii::Fence& command_finished_fense(
      cur_frame.GetCommandFinishedFence());
  if (gpu_->WaitForFence(command_finished_fense) != vk::Result::eSuccess) {
    THROW("Fail to wait for fences.");
  }
  gpu_->ResetFence(command_finished_fense);

  const vk::raii::CommandBuffer& command_buffer{
      cur_frame.GetActiveCommandBuffer()};
  command_buffer.reset({});
  command_buffer.begin({});

  return command_buffer;
}

void Context::TarversePasses(const vk::raii::CommandBuffer& command_buffer) {
  // Set viewport and scissor.
  command_buffer.setViewport(0, viewport_);
  command_buffer.setScissor(0, scissor_);

  for (u32 i{0}; i < passes_.size(); ++i) {
    // Begin render pass.
    const std::unique_ptr<rd::Pass>& pass{passes_[i]};
    const vk::raii::RenderPass& render_pass{pass->GetRenderPass()};
    const vk::raii::Framebuffer& framebuffer{
        pass->GetFramebuffer(active_frame_index_)};
    const vk::Rect2D& render_area{pass->GetRenderArea()};
    const std::vector<vk::ClearValue> clear_values{pass->GetClearValues()};

    vk::RenderPassBeginInfo render_pass_bi{*render_pass, *framebuffer,
                                           render_area, clear_values};
    command_buffer.beginRenderPass(render_pass_bi,
                                   vk::SubpassContents::eInline);

    // Tarverse subpasses.
    const std::vector<std::unique_ptr<rd::Subpass>>& subpasses{
        pass->GetSubpasses()};
    for (u32 j{0}; j < subpasses.size(); ++j) {
      const std::unique_ptr<rd::Subpass>& subpass{subpasses[j]};

      // Next subpass.
      if (j > 0) {
        command_buffer.nextSubpass({});
      }

      // Traverse draw elements.
      const std::vector<DrawElement>& draw_elements{subpass->GetDrawElements()};

      vk::Pipeline prev_pipeline{nullptr};
      for (const DrawElement& draw_element : draw_elements) {
        // Bind pipeline.
        vk::Pipeline pipeline{draw_element.pipeline};
        if (prev_pipeline != pipeline) {
          command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                      pipeline);
          prev_pipeline = pipeline;
        }

        // Bind descriptor sets.
        command_buffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, draw_element.pipeline_layout, 0,
            draw_element.descriptor_sets, nullptr);

        // Bind vertex buffers.
        const auto& vertex_buffer_attributes{
            draw_element.vertex_buffer_attributes};
        for (const auto& location_vertex_buffer : vertex_buffer_attributes) {
          vk::Buffer vertex_buffer{location_vertex_buffer.first};
          const sg::VertexAttribute& vertex_attribute{
              location_vertex_buffer.second};

          command_buffer.bindVertexBuffers(vertex_attribute.location,
                                           vertex_buffer,
                                           vertex_attribute.offset);
        }

        // Bind index buffer and draw indexed.
        if (!draw_element.has_index) {
          command_buffer.draw(draw_element.vertex_count, 1, 0, 0);
        } else {
          const auto& index_buffer_attribute{
              draw_element.index_buffer_attribute};

          vk::Buffer index_buffer{index_buffer_attribute.first};
          const sg::IndexAttribute& index_attribute{
              index_buffer_attribute.second};

          command_buffer.bindIndexBuffer(index_buffer, index_attribute.offset,
                                         index_attribute.index_type);

          command_buffer.drawIndexed(index_attribute.count, 1, 0, 0, 0);
        }
      }
    }

    // End render pass.
    command_buffer.endRenderPass();
  }
}

void Context::End(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.end();

  auto& cur_frame{frames_[active_frame_index_]};
  vk::PipelineStageFlags wait_pipeline_stage{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::SubmitInfo submit_info{*(acquired_semaphores_[acquired_semaphore_index]),
                             wait_pipeline_stage, *command_buffer,
                             *(cur_frame.GetRenderFinishedSemphore())};

  const vk::raii::Queue& graphics_queue{gpu_->GetGraphicsQueue()};
  graphics_queue.submit(submit_info, *(cur_frame.GetCommandFinishedFence()));

  vk::PresentInfoKHR present_info{*(cur_frame.GetRenderFinishedSemphore()),
                                  *swapchain_, active_frame_index_};

  const vk::raii::Queue& present_queue{gpu_->GetPresentQueue()};

  vk::Result result{present_queue.presentKHR(present_info)};
  if (result != vk::Result::eSuccess) {
    THROW("Fail to present.");
  }

  acquired_semaphore_index =
      (acquired_semaphore_index + 1) % acquired_semaphores_.size();
}

void Context::CreateSwapchain() {
  // Image count.
  const vk::SurfaceCapabilitiesKHR& surface_capabilities{
      gpu_->GetSurfaceCapabilities()};
  swapchain_info_.image_count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      swapchain_info_.image_count > surface_capabilities.maxImageCount) {
    swapchain_info_.image_count = surface_capabilities.maxImageCount;
  }

  // Format and color space.
  const std::vector<vk::SurfaceFormatKHR>& surface_formats{
      gpu_->GetSurfaceFormats()};

  vk::SurfaceFormatKHR picked_format{surface_formats[0]};

  std::vector<vk::Format> requested_formats{vk::Format::eR8G8B8A8Srgb,
                                            vk::Format::eB8G8R8A8Srgb};
  vk::ColorSpaceKHR requested_color_space{vk::ColorSpaceKHR::eSrgbNonlinear};
  for (const auto& requested_format : requested_formats) {
    auto it{std::find_if(surface_formats.begin(), surface_formats.end(),
                         [requested_format, requested_color_space](
                             const vk::SurfaceFormatKHR& f) {
                           return (f.format == requested_format) &&
                                  (f.colorSpace == requested_color_space);
                         })};
    if (it != surface_formats.end()) {
      picked_format = *it;
      break;
    }
  }

  swapchain_info_.color_format = picked_format.format;
  swapchain_info_.color_space = picked_format.colorSpace;

  // Extent.
  if (surface_capabilities.currentExtent.width ==
      std::numeric_limits<u32>::max()) {
    i32 width{0};
    i32 height{0};
    window_->GetFramebufferSize(&width, &height);

    swapchain_info_.extent.width = std::clamp(
        static_cast<u32>(width), surface_capabilities.minImageExtent.width,
        surface_capabilities.maxImageExtent.width);
    swapchain_info_.extent.height = std::clamp(
        static_cast<u32>(height), surface_capabilities.minImageExtent.height,
        surface_capabilities.maxImageExtent.height);
  } else {
    swapchain_info_.extent = surface_capabilities.currentExtent;
  }

  // Present mode.
  std::vector<vk::PresentModeKHR> present_modes{gpu_->GetSurfacePresentModes()};

  std::vector<vk::PresentModeKHR> requested_present_modes{
      vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo,
      vk::PresentModeKHR::eImmediate};

  vk::PresentModeKHR picked_mode{vk::PresentModeKHR::eImmediate};
  for (const auto& requested_present_mode : requested_present_modes) {
    auto it{std::find_if(present_modes.begin(), present_modes.end(),
                         [requested_present_mode](const vk::PresentModeKHR& p) {
                           return p == requested_present_mode;
                         })};
    if (it != present_modes.end()) {
      picked_mode = *it;
      break;
    }
  }
  swapchain_info_.present_mode = picked_mode;

  // Create swapchain
  vk::SwapchainCreateInfoKHR swapchain_ci{
      {},
      {},
      swapchain_info_.image_count,
      swapchain_info_.color_format,
      swapchain_info_.color_space,
      swapchain_info_.extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {},
      surface_capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      swapchain_info_.present_mode,
      VK_TRUE,
      {}};

  u32 graphics_queue_index{gpu_->GetGraphicsQueueIndex()};
  u32 present_queue_index{gpu_->GetPresentQueueIndex()};

  if (graphics_queue_index != present_queue_index) {
    u32 queue_family_indices[2]{graphics_queue_index, present_queue_index};
    swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queue_family_indices;
  }

  swapchain_ = gpu_->CreateSwapchain(swapchain_ci);

  swapchain_images_ = swapchain_.getImages();
}

void Context::CreateFrames() {
  frame_count_ = swapchain_images_.size();
  for (u32 i{0}; i < frame_count_; ++i) {
    frames_.emplace_back(gpu_);
  }
}

void Context::CreateAcquiredSemphores() {
  acquired_semaphores_.reserve(frame_count_);
  vk::SemaphoreCreateInfo semaphore_ci;
  for (u32 i = 0; i < frame_count_; ++i) {
    acquired_semaphores_.emplace_back(gpu_->CreateSemaphore0(semaphore_ci));
  }
}

void Context::CreateViewportAndScissor() {
  u32 target_width{swapchain_info_.extent.width};
  u32 target_height{swapchain_info_.extent.height};

  viewport_ = vk::Viewport{0.0f,
                           0.0f,
                           static_cast<f32>(target_width),
                           static_cast<f32>(target_height),
                           0.0f,
                           1.0f};
  scissor_ =
      vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{target_width, target_height}};
}

void Context::CreatePasses() {
  // Swapchain pass.
  std::unique_ptr<Pass> swapchain_pass{std::make_unique<SwapchainPass>(
      asset_, gpu_, scene_graph_, frames_, swapchain_info_, swapchain_images_)};

  passes_.push_back(std::move(swapchain_pass));
}

}  // namespace rd

}  // namespace luka
