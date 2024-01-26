// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/pass.h"

namespace luka {

namespace rd {

const vk::raii::RenderPass& Pass::GetRenderPass() const { return render_pass_; }

const vk::Rect2D& Pass::GetRenderArea() const { return render_area_; }

const std::vector<vk::ClearValue>& Pass::GetClearValues() const {
  return clear_values_;
}

const std::vector<std::unique_ptr<Subpass>>& Pass::GetSubpasses() const {
  return subpasses_;
}

}  // namespace rd

}  // namespace luka
