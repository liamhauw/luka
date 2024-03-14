// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/rendering.h"

namespace luka {

Rendering::Rendering(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu,
                     std::shared_ptr<Asset> asset,
                     std::shared_ptr<Camera> camera)
    : window_{window},
      gpu_{gpu},
      asset_{asset},
      camera_{camera},
      context_{window_, gpu_, asset_, camera_} {}

Rendering::~Rendering() { gpu_.reset(); }

void Rendering::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
    context_.Resize();
  }

  context_.Render();
}

}  // namespace luka
