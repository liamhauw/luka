// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include "resource/config/config.h"
#include "resource/window/window.h"

namespace luka {

class EditorUi {
 public:
  EditorUi(std::shared_ptr<Config> config, std::shared_ptr<Window> window);

  void Tick();

 private:
  void CreateUi();

  std::shared_ptr<Config> config_;
  std::shared_ptr<Window> window_;
};

}  // namespace luka
