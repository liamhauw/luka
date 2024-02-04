// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/frame.h"
#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

class Pass {
 public:
  Pass(std::shared_ptr<Gpu> gpu, std::vector<Frame>& frames,
       u32 attachment_count, const std::vector<u32>& color_attachment_indices,
       const std::vector<u32>& resolve_attachment_indices,
       u32 depth_stencil_attachment_index);
  virtual ~Pass() = default;

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
  std::shared_ptr<Gpu> gpu_;
  std::vector<Frame>* frames_;

  u32 attachment_count_;
  std::vector<u32> color_attachment_indices_;
  std::vector<u32> resolve_attachment_indices_;
  u32 depth_stencil_attachment_index_;

  vk::raii::RenderPass render_pass_{nullptr};
  u32 images_index_{0};
  u32 image_views_index_{0};
  u32 framebuffer_index_{0};
  vk::Rect2D render_area_;
  std::vector<vk::ClearValue> clear_values_;
  std::vector<std::unique_ptr<Subpass>> subpasses_;
};

}  // namespace rd

}  // namespace luka
