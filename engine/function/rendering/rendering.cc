/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/
#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering()
    : window_{gContext.window}, gpu_{std::make_unique<Gpu>(window_)} {}

Rendering::~Rendering() { gpu_.reset(); }

void Rendering::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
    gpu_->Resize();
  }
}

}  // namespace luka
