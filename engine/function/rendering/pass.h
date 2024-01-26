// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

class Pass {
 public:
  const vk::raii::RenderPass& GetRenderPass() const;
  const vk::Rect2D& GetRenderArea() const;
  const std::vector<vk::ClearValue>& GetClearValues() const;
  const std::vector<std::unique_ptr<Subpass>>& GetSubpasses() const;

 private:
  vk::raii::RenderPass render_pass_{nullptr};
  vk::Rect2D render_area_;
  std::vector<vk::ClearValue> clear_values_;
  std::vector<std::unique_ptr<Subpass>> subpasses_;
};

}  // namespace rd

}  // namespace luka
