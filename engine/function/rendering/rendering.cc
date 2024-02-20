// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

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
      context_{asset_, window_, gpu_, scene_graph_} {}

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

  context_.Draw();
}

}  // namespace luka
