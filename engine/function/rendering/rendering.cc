/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering source file.
*/

#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering()
    : window_{gContext.window},
      instance_{{}, window_->GetRequiredInstanceExtension()} {}

void Rendering::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
  }
}

}  // namespace luka
