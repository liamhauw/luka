// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/graphics/graphics.h"

#include "core/log.h"

namespace luka {

Graphics::Graphics(std::shared_ptr<Config> config,
                   std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu,
                   std::shared_ptr<Asset> asset, std::shared_ptr<Camera> camera,
                   std::shared_ptr<FunctionUi> function_ui)
    : config_{config},
      window_{window},
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
  std::tie(result, frame_index_) = swapchain_->acquireNextImage(
      UINT64_MAX,
      *(image_acquired_semaphores_[image_acquired_semaphore_index_]), nullptr);

  const vk::raii::Fence& command_finished_fence{
      command_finished_fences_[frame_index_]};
  gpu_->WaitForFence(command_finished_fence);

  gpu_->ResetFence(command_finished_fence);

  const vk::raii::CommandBuffer& command_buffer{
      command_buffers_[frame_index_][0]};
  command_buffer.reset({});
  command_buffer.begin({});

  return command_buffer;
}

void Graphics::End(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.end();

  vk::PipelineStageFlags wait_pipeline_stage{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::SubmitInfo submit_info{
      *(image_acquired_semaphores_[image_acquired_semaphore_index_]),
      wait_pipeline_stage, *command_buffer,
      *(render_finished_semaphores_[frame_index_])};

  const vk::raii::Queue& graphics_queue{gpu_->GetGraphicsQueue()};
  graphics_queue.submit(submit_info, *(command_finished_fences_[frame_index_]));

  vk::PresentInfoKHR present_info{*(render_finished_semaphores_[frame_index_]),
                                  **swapchain_, frame_index_};

  const vk::raii::Queue& present_queue{gpu_->GetPresentQueue()};

  vk::Result result{present_queue.presentKHR(present_info)};
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    THROW("Fail to present.");
  }

  image_acquired_semaphore_index_ =
      (image_acquired_semaphore_index_ + 1) % frame_count_;
}

void Graphics::TarversePasses(const vk::raii::CommandBuffer& command_buffer) {
  // Set viewport and scissor.
  command_buffer.setViewport(0, viewport_);
  command_buffer.setScissor(0, scissor_);

  for (const auto& pass : passes_) {
#ifndef NDEBUG
    gpu_->BeginLabel(command_buffer, "Pass " + pass.GetName(),
                     {0.549F, 0.478F, 0.663F, 1.0F});
#endif
    // Begin render pass.
    command_buffer.beginRenderPass(pass.GetRenderPassBeginInfo(frame_index_),
                                   vk::SubpassContents::eInline);

    // Tarverse subpasses.
    const std::vector<gs::Subpass>& subpasses{pass.GetSubpasses()};
    for (u32 i{0}; i < subpasses.size(); ++i) {
      const gs::Subpass& subpass{subpasses[i]};
#ifndef NDEBUG
      gpu_->BeginLabel(command_buffer, "Subpass " + subpass.GetName(),
                       {0.443F, 0.573F, 0.745F, 1.0F});
#endif

      // Next subpass.
      if (i > 0) {
        command_buffer.nextSubpass({});
      }

      // Traverse draw elements.
      const std::vector<gs::DrawElement>& draw_elements{
          subpass.GetDrawElements()};

      const vk::raii::PipelineLayout* prev_pipeline_layout{nullptr};
      const vk::raii::Pipeline* prev_pipeline{nullptr};
      const vk::raii::DescriptorSet* prev_descriptor_set{nullptr};
      for (const gs::DrawElement& draw_element : draw_elements) {
        // Push constants.
        const vk::raii::PipelineLayout* pipeline_layout{
            draw_element.pipeline_layout};
        if (subpass.HasPushConstant() &&
            prev_pipeline_layout != pipeline_layout) {
          subpass.PushConstants(command_buffer, **pipeline_layout);
          prev_pipeline_layout = pipeline_layout;
        }

        // Bind pipeline.
        const vk::raii::Pipeline* pipeline{draw_element.pipeline};
        if (prev_pipeline != pipeline) {
          command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                      **pipeline);
          prev_pipeline = pipeline;
        }

        // Bind bindless descriptor sets;
        const vk::raii::DescriptorSet& bindless_descriptor_set{
            subpass.GetBindlessDescriptorSet()};
        if (prev_descriptor_set != &bindless_descriptor_set) {
          command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                            **pipeline_layout, 0,
                                            *bindless_descriptor_set, nullptr);
          prev_descriptor_set = &bindless_descriptor_set;
        }

        // Bind normal descriptor sets.
        const vk::raii::DescriptorSets& descriptor_sets{
            draw_element.descriptor_sets[frame_index_]};
        if (!descriptor_sets.empty()) {
          std::vector<vk::DescriptorSet> vk_descriptor_sets;
          for (const auto& descriptor_set : descriptor_sets) {
            vk_descriptor_sets.push_back(*(descriptor_set));
          }
          command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                            **pipeline_layout, 1,
                                            vk_descriptor_sets, nullptr);
        }

        // Draw.
        if (draw_element.has_primitive) {
          const std::vector<gs::DrawElmentVertexInfo> vertex_infos{
              draw_element.vertex_infos};

          for (const auto& vertex_info : vertex_infos) {
            command_buffer.bindVertexBuffers(
                vertex_info.location, vertex_info.buffers, vertex_info.offsets);
          }

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
  vk::SemaphoreCreateInfo semaphore_ci;
  vk::FenceCreateInfo fence_ci{vk::FenceCreateFlagBits::eSignaled};
  for (u32 i{0}; i < frame_count_; ++i) {
    image_acquired_semaphores_.push_back(gpu_->CreateSemaphoreLuka(
        semaphore_ci, "image_acquired_" + std::to_string(i)));
    render_finished_semaphores_.push_back(gpu_->CreateSemaphoreLuka(
        semaphore_ci, "render_finished_" + std::to_string(i)));
    command_finished_fences_.push_back(
        gpu_->CreateFence(fence_ci, "command_finished_" + std::to_string(i)));
  }
}

void Graphics::CreateCommandObjects() {
  vk::CommandPoolCreateInfo command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      gpu_->GetGraphicsQueueIndex()};
  vk::CommandBufferAllocateInfo command_buffer_ai{
      nullptr, vk::CommandBufferLevel::ePrimary, 1};
  for (u32 i{0}; i < frame_count_; ++i) {
    command_pools_.push_back(gpu_->CreateCommandPool(
        command_pool_ci, "graphics_" + std::to_string(i)));

    command_buffer_ai.commandPool = *(command_pools_.back());
    command_buffers_.push_back(gpu_->AllocateCommandBuffers(
        command_buffer_ai, "graphics_" + std::to_string(i)));
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
  u32 frame_graph_index{config_->GetFrameGraphIndex()};
  const ast::FrameGraph& frame_graph{asset_->GetFrameGraph(frame_graph_index)};
  const std::vector<ast::Pass>& ast_passes{frame_graph.GetPasses()};

  shared_image_views_.resize(frame_count_);

  for (u32 i{0}; i < ast_passes.size(); ++i) {
    passes_.emplace_back(gpu_, asset_, camera_, function_ui_, frame_count_,
                         *swapchain_info_, swapchain_images_, ast_passes, i,
                         shared_image_views_);
  }
}

}  // namespace luka
