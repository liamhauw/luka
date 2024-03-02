// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

struct SwapchainInfo {
  u32 image_count;
  vk::Format color_format;
  vk::ColorSpaceKHR color_space;
  vk::Extent2D extent;
  vk::PresentModeKHR present_mode;
  vk::Format depth_stencil_format_{vk::Format::eD32Sfloat};
};

class Pass {
 public:
  Pass(u32 attachment_count, const std::vector<u32>& color_attachment_indices,
       const std::vector<u32>& resolve_attachment_indices,
       u32 depth_stencil_attachment_index);
  virtual ~Pass() = default;

  virtual void Resize(const SwapchainInfo& swapchain_info,
                      const std::vector<vk::Image>& swapchain_images) {}

  u32 GetAttachmentCount() const;
  const std::vector<u32>& GetColorAttachmentIndices() const;
  const std::vector<u32>& GetResloveAttachmentIndices() const;
  u32 GetDepthStencilAttachmentIndex() const;

  const vk::raii::RenderPass& GetRenderPass() const;
  const vk::raii::Framebuffer& GetFramebuffer(u32 frame_index) const;
  const vk::Rect2D& GetRenderArea() const;
  const std::vector<vk::ClearValue>& GetClearValues() const;
  const std::vector<std::unique_ptr<Subpass>>& GetSubpasses() const;

 protected:
  virtual void CreateRenderPass() = 0;
  virtual void CreateFramebuffers() = 0;
  virtual void CreateRenderArea() = 0;
  virtual void CreateClearValues() = 0;
  virtual void CreateSubpasses() = 0;

  u32 attachment_count_;
  std::vector<u32> color_attachment_indices_;
  std::vector<u32> resolve_attachment_indices_;
  u32 depth_stencil_attachment_index_;

  vk::raii::RenderPass render_pass_{nullptr};

  std::vector<std::vector<gpu::Image>> images_;
  std::vector<std::vector<vk::raii::ImageView>> image_views_;
  std::vector<vk::raii::Framebuffer> framebuffers_;

  vk::Rect2D render_area_;

  std::vector<vk::ClearValue> clear_values_;

  std::vector<std::unique_ptr<Subpass>> subpasses_;
};

}  // namespace rd

}  // namespace luka
