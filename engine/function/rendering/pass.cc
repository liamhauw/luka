// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/pass.h"

namespace luka {

namespace rd {

Pass::Pass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
           std::shared_ptr<SceneGraph> scene_graph, std::vector<Frame>& frames,
           u32 attachment_count,
           const std::vector<u32>& color_attachment_indices,
           const std::vector<u32>& resolve_attachment_indices,
           u32 depth_stencil_attachment_index)
    : asset_{asset},
      gpu_{gpu},
      scene_graph_{scene_graph},
      frames_{&frames},
      attachment_count_{attachment_count},
      color_attachment_indices_{color_attachment_indices},
      resolve_attachment_indices_{resolve_attachment_indices},
      depth_stencil_attachment_index_{depth_stencil_attachment_index} {}

u32 Pass::GetAttachmentCount() const { return attachment_count_; }

const std::vector<u32>& Pass::GetColorAttachmentIndices() const {
  return color_attachment_indices_;
}

const std::vector<u32>& Pass::GetResloveAttachmentIndices() const {
  return resolve_attachment_indices_;
}

u32 Pass::GetDepthStencilAttachmentIndex() const {
  return depth_stencil_attachment_index_;
}

const vk::raii::RenderPass& Pass::GetRenderPass() const { return render_pass_; }

const vk::raii::Framebuffer& Pass::GetFramebuffer(u32 frame_index) const {
  return (*frames_)[frame_index].GetFramebuffer(framebuffer_index_);
}

const vk::Rect2D& Pass::GetRenderArea() const { return render_area_; }

const std::vector<vk::ClearValue>& Pass::GetClearValues() const {
  return clear_values_;
}

const std::vector<std::unique_ptr<Subpass>>& Pass::GetSubpasses() const {
  return subpasses_;
}

}  // namespace rd

}  // namespace luka
