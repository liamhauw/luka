// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-swapchain_info.format off
#include "platform/pch.h"
// clang-swapchain_info.format on

#include "function/rendering/rendering.h"

namespace luka {

Rendering::Rendering(std::shared_ptr<Asset> asset,
                     std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu,
                     std::shared_ptr<SceneGraph> scene_graph)
    : asset_{asset},
      window_{window},
      gpu_{gpu},
      scene_graph_{scene_graph},
      context_{window_, gpu_} {}

Rendering::~Rendering() { gpu_.reset(); }

void Rendering::Tick() {
  // Iconify.
  if (window_->GetIconified()) {
    return;
  }

  // Resize.
  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
    context_.Resize();
  }

  // Begin frame.
  const vk::raii::CommandBuffer& command_buffer{context_.Begin()};
  u32 active_frame_index{context_.GetActiveFrameIndex()};

  // Tarverse passes.
  const std::vector<std::unique_ptr<rd::Pass>>& passes{context_.GetPasses()};
  for (u32 i{0}; i < passes.size(); ++i) {
    // Begin render pass.
    const std::unique_ptr<rd::Pass>& pass{passes[i]};
    const vk::raii::RenderPass& render_pass{pass->GetRenderPass()};
    const vk::raii::Framebuffer& framebuffer{
        pass->GetFramebuffer(active_frame_index)};
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

      // Bind pipeline.
      const vk::raii::Pipeline& pipeline{subpass->GetPipeline()};
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

      // Set viewport and scissor.
      const vk::Viewport& viewport{subpass->GetViewport()};
      const vk::Rect2D& scissor{subpass->GetScissor()};
      command_buffer.setViewport(0, viewport);
      command_buffer.setScissor(0, scissor);

      // Draw.
    }

    // End render pass.
    command_buffer.endRenderPass();
  }

  // End frame.
  context_.End(command_buffer);
}

}  // namespace luka
