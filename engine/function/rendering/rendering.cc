/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering source file.
*/

#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering() : window_{gContext.window} {
  gpu_ = std::make_unique<Gpu>();
}

void Rendering::Tick() {
  if (!window_->GetIconified()) {
    gpu_->NewFrame();
  }
  if (window_->GetResized()) {
    gpu_->Resize();
    window_->SetResized(false);
  }
}

void Rendering::Terminate() { gpu_.reset(); }

}  // namespace luka
