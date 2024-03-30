// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/graphics/graphics.h"

#include "core/log.h"

namespace luka {

Graphics::Graphics(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu,
                   std::shared_ptr<Asset> asset, std::shared_ptr<Camera> camera,
                   std::shared_ptr<FunctionUi> function_ui)
    : window_{window},
      gpu_{gpu},
      asset_{asset},
      camera_{camera},
      function_ui_{function_ui} {
  GetSwapchain();
  CreateSyncObjects();
  CreateCommandObjects();
  CreateViewportAndScissor();
  CreatePasses();
}

Graphics::~Graphics() { gpu_->WaitIdle(); }

void Graphics::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
    Resize();
  }

  Render();
}

void Graphics::Resize() {
  gpu_->WaitIdle();
  GetSwapchain();
  CreateViewportAndScissor();
  for (auto& pass : passes_) {
    pass.Resize(*swapchain_info_, swapchain_images_);
  }
}

void Graphics::Render() {
  const vk::raii::CommandBuffer& command_buffer{Begin()};
  TarversePasses(command_buffer);
  End(command_buffer);
}

const vk::raii::CommandBuffer& Graphics::Begin() {
  vk::Result result;
  std::tie(result, active_frame_index_) = swapchain_->acquireNextImage(
      UINT64_MAX, *(acquired_semaphores_[acquired_semaphore_index_]), nullptr);

  const vk::raii::Fence& command_finished_fence{
      command_finished_fences_[active_frame_index_]};
  gpu_->WaitForFence(command_finished_fence);

  gpu_->ResetFence(command_finished_fence);

  const vk::raii::CommandBuffer& command_buffer{
      command_buffers_[active_frame_index_][0]};
  command_buffer.reset({});
  command_buffer.begin({});

  return command_buffer;
}

void Graphics::End(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.end();

  vk::PipelineStageFlags wait_pipeline_stage{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::SubmitInfo submit_info{
      *(acquired_semaphores_[acquired_semaphore_index_]), wait_pipeline_stage,
      *command_buffer, *(render_finished_semaphores_[active_frame_index_])};

  const vk::raii::Queue& graphics_queue{gpu_->GetGraphicsQueue()};
  graphics_queue.submit(submit_info,
                        *(command_finished_fences_[active_frame_index_]));

  vk::PresentInfoKHR present_info{
      *(render_finished_semaphores_[active_frame_index_]), **swapchain_,
      active_frame_index_};

  const vk::raii::Queue& present_queue{gpu_->GetPresentQueue()};

  vk::Result result{present_queue.presentKHR(present_info)};
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    THROW("Fail to present.");
  }

  acquired_semaphore_index_ = (acquired_semaphore_index_ + 1) % frame_count_;
}

void Graphics::TarversePasses(const vk::raii::CommandBuffer& command_buffer) {
  // Set viewport and scissor.
  command_buffer.setViewport(0, viewport_);
  command_buffer.setScissor(0, scissor_);

  for (u32 i{0}; i < passes_.size(); ++i) {
    const gs::Pass& pass{passes_[i]};
#ifndef NDEBUG
    gpu_->BeginLabel(command_buffer, "Pass " + pass.GetName(),
                     {0.549F, 0.478F, 0.663F, 1.0F});
#endif
    // Begin render pass.
    const vk::raii::RenderPass& render_pass{pass.GetRenderPass()};
    const vk::raii::Framebuffer& framebuffer{
        pass.GetFramebuffer(active_frame_index_)};
    const vk::Rect2D& render_area{pass.GetRenderArea()};
    const std::vector<vk::ClearValue> clear_values{pass.GetClearValues()};

    vk::RenderPassBeginInfo render_pass_bi{*render_pass, *framebuffer,
                                           render_area, clear_values};
    command_buffer.beginRenderPass(render_pass_bi,
                                   vk::SubpassContents::eInline);

    // Tarverse subpasses.
    const std::vector<gs::Subpass>& subpasses{pass.GetSubpasses()};
    for (u32 j{0}; j < subpasses.size(); ++j) {
      const gs::Subpass& subpass{subpasses[j]};
#ifndef NDEBUG
      gpu_->BeginLabel(command_buffer, "Subpass " + subpass.GetName(),
                       {0.443F, 0.573F, 0.745F, 1.0F});
#endif

      // Next subpass.
      if (j > 0) {
        command_buffer.nextSubpass({});
      }

      // Traverse draw elements.
      const std::vector<gs::DrawElement>& draw_elements{
          subpass.GetDrawElements()};

      vk::Pipeline prev_pipeline{nullptr};
      vk::PipelineLayout prev_pipeline_layout{nullptr};
      vk::DescriptorSet prev_descriptor_set{nullptr};
      for (const gs::DrawElement& draw_element : draw_elements) {
        // Bind pipeline.
        vk::Pipeline pipeline{draw_element.pipeline};
        if (prev_pipeline != pipeline) {
          command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                      pipeline);
          prev_pipeline = pipeline;
        }

        // Update subpass uniform buffer.
        vk::PipelineLayout pipeline_layout{draw_element.pipeline_layout};
        if (subpass.HasPushConstant() &&
            prev_pipeline_layout != pipeline_layout) {
          subpass.PushConstants(command_buffer, pipeline_layout);
          prev_pipeline_layout = pipeline_layout;
        }

        // Bind bindless descriptor sets;
        if (draw_element.has_primitive) {
          vk::DescriptorSet bindless_descriptor_set{
              *(subpass.GetBindlessDescriptorSet())};
          if (prev_descriptor_set != bindless_descriptor_set) {
            command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                              pipeline_layout, 0,
                                              bindless_descriptor_set, nullptr);
            prev_descriptor_set = bindless_descriptor_set;
          }
        }

        // Bind normal descriptor sets.
        std::vector<vk::DescriptorSet> descriptor_sets;
        for (const auto& descriptor_set :
             draw_element.descriptor_sets[active_frame_index_]) {
          descriptor_sets.push_back(*(descriptor_set));
        }
        if (!descriptor_sets.empty()) {
          command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                            pipeline_layout, 1, descriptor_sets,
                                            nullptr);
        }

        // Bind vertex buffers and draw.
        if (draw_element.has_primitive) {
          const auto& location_vertex_attributes{
              draw_element.location_vertex_attributes};

          std::vector<vk::Buffer> vertex_buffers(
              location_vertex_attributes.size());
          std::vector<u64> offsets(location_vertex_attributes.size());

          for (const auto& location_vertex_attribute :
               location_vertex_attributes) {
            u32 location{location_vertex_attribute.first};
            const ast::sc::VertexAttribute* vertex_attribute{
                location_vertex_attribute.second};

            vertex_buffers[location] = *(vertex_attribute->buffer);
            offsets[location] = vertex_attribute->offset;
          }
          command_buffer.bindVertexBuffers(0, vertex_buffers, offsets);

          if (!draw_element.has_index) {
            command_buffer.draw(draw_element.vertex_count, 1, 0, 0);
          } else {
            const ast::sc::IndexAttribute* index_attribute{
                draw_element.index_attribute};

            command_buffer.bindIndexBuffer(*(index_attribute->buffer),
                                           index_attribute->offset,
                                           index_attribute->index_type);

            command_buffer.drawIndexed(index_attribute->count, 1, 0, 0, 0);
          }
        } else {
          command_buffer.draw(3, 1, 0, 0);
        }
      }

      if (pass.HasUi()) {
        function_ui_->Render(command_buffer);
      }
#ifndef NDEBUG
      gpu_->EndLabel(command_buffer);
#endif
    }

    // End render pass.
    command_buffer.endRenderPass();
#ifndef NDEBUG
    gpu_->EndLabel(command_buffer);
#endif
  }
}

void Graphics::GetSwapchain() {
  swapchain_info_ = &(function_ui_->GetSwapchainInfo());
  swapchain_ = &(function_ui_->GetSwapchain());
  swapchain_images_ = swapchain_->getImages();
  frame_count_ = swapchain_images_.size();
}

void Graphics::CreateSyncObjects() {
  vk::FenceCreateInfo fence_ci{vk::FenceCreateFlagBits::eSignaled};
  vk::SemaphoreCreateInfo semaphore_ci;
  for (u32 i{0}; i < frame_count_; ++i) {
    acquired_semaphores_.push_back(gpu_->CreateSemaphoreLuka(semaphore_ci));
    render_finished_semaphores_.push_back(
        gpu_->CreateSemaphoreLuka(semaphore_ci));
    command_finished_fences_.push_back(gpu_->CreateFence(fence_ci));
  }
}

void Graphics::CreateCommandObjects() {
  vk::CommandPoolCreateInfo command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      gpu_->GetGraphicsQueueIndex()};
  vk::CommandBufferAllocateInfo command_buffer_ai{
      nullptr, vk::CommandBufferLevel::ePrimary, 1};
  for (u32 i{0}; i < frame_count_; ++i) {
    command_pools_.push_back(gpu_->CreateCommandPool(command_pool_ci));

    command_buffer_ai.commandPool = *(command_pools_.back());
    command_buffers_.push_back(gpu_->AllocateCommandBuffers(command_buffer_ai));
  }
}

void Graphics::CreateViewportAndScissor() {
  u32 target_width{swapchain_info_->extent.width};
  u32 target_height{swapchain_info_->extent.height};

  viewport_ = vk::Viewport{0.0F,
                           0.0F,
                           static_cast<f32>(target_width),
                           static_cast<f32>(target_height),
                           0.0F,
                           1.0F};
  scissor_ =
      vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{target_width, target_height}};
}

void Graphics::CreatePasses() {
  const ast::FrameGraph& frame_graph{asset_->GetFrameGraph(0)};
  const std::vector<ast::Pass>& ast_passes{frame_graph.GetPasses()};

  shared_image_views_.resize(frame_count_);

  for (u32 i{0}; i < ast_passes.size(); ++i) {
    passes_.emplace_back(gpu_, asset_, camera_, function_ui_, frame_count_,
                         *swapchain_info_, swapchain_images_, ast_passes, i,
                         shared_image_views_);
  }
}

}  // namespace luka
