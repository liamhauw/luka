// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on
#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering() : gpu_{gContext.gpu} {}

void Rendering::Tick() {
  if (gContext.window->GetIconified()) {
    return;
  }

  const vk::raii::CommandBuffer& cur_command_buffer{gpu_->BeginFrame()};

  ImDrawData* draw_data{ImGui::GetDrawData()};
  ImGui_ImplVulkan_RenderDrawData(
      draw_data, static_cast<VkCommandBuffer>(*cur_command_buffer));
  

  gpu_->EndFrame(cur_command_buffer);
}

}  // namespace luka
