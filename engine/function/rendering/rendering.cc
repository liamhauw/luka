/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering source file.
*/

#include "function/rendering/rendering.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Rendering::Rendering()
    : gpu_{std::make_unique<Gpu>()}, window_{gContext.window} {
  std::vector<const char*> window_required_instance_extensions{
      window_->GetRequiredInstanceExtension()};
  gpu_->MakeInstance(window_required_instance_extensions);

  VkSurfaceKHR surface;
  window_->CreateWindowSurface(gpu_->GetInstance(), &surface);
  gpu_->MakeSurface(surface);

  gpu_->MakePhysicalDevice();

  gpu_->MakeDevice();

  int width{0};
  int height{0};
  window_->GetFramebufferSize(&width, &height);
  gpu_->MakeSwapchain(width, height);

  gpu_->MakeCommandObjects();

  gpu_->MakeSyncObjects();

  gpu_->MakeDepthImage();

  gpu_->MakeRenderPass();

  gpu_->MakeFramebuffer();

  gpu_->MakeGraphicsPipeline(gContext.asset->GetVertexShaderBuffer(),
                             gContext.asset->GetFragmentShaderBuffer());
}

Rendering::~Rendering() { gpu_.reset(); }

void Rendering::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);

    int width{0};
    int height{0};
    window_->GetFramebufferSize(&width, &height);
    gpu_->Resize(width, height);
  }

  gpu_->BeginFrame();

  gpu_->EndFrame();
}

}  // namespace luka
