// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

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
      context_{window_, gpu_},
      pipeline_{asset_, scene_graph_, context_} {}

Rendering::~Rendering() { gpu_.reset(); }

void Rendering::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
    context_.Resize();
  }

  const vk::raii::CommandBuffer& command_buffer{context_.Begin()};

  pipeline_.Draw(command_buffer, context_.GetActiveFrame().GetTarget());

  context_.End(command_buffer);
}

}  // namespace luka
