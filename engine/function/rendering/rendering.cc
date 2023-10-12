/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering source file.
*/

#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering()
    : asset_{gContext.asset},
      window_{gContext.window},
      gpu_{std::make_unique<Gpu>(window_)} {
  gpu_->MakeGraphicsPipeline(asset_->GetVertexShaderBuffer(),
                             asset_->GetFragmentShaderBuffer());
}

Rendering::~Rendering() { gpu_.reset(); }

void Rendering::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
    gpu_->Resize();
  }

  gpu_->BeginFrame();

  gpu_->EndFrame();
}

}  // namespace luka
