// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on
#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering()
    : gpu_{std::make_unique<Gpu>()}, camera_{std::make_unique<Camera>()} {}

Rendering::~Rendering() {
  camera_.reset();
  gpu_.reset();
}

void Rendering::Tick() {
  if (gContext.load) {
    camera_->LookAt(gContext.config->GetCameraFrom(),
                    gContext.config->GetCameraTo());

    gContext.load = false;
  }

  if (gContext.window->GetIconified()) {
    return;
  }

  if (gContext.window->GetFramebufferResized()) {
    gContext.window->SetFramebufferResized(false);
    gpu_->Resize();
  }

  gpu_->BeginFrame();

  const vk::raii::CommandBuffer& command_buffer{gpu_->GetCommandBuffer()};
  vk::CommandBufferBeginInfo command_buffer_bi{
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

  gpu_->EndFrame();
}

}  // namespace luka
