// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

namespace luka {

class EditorUi {
 public:
  EditorUi();

  void Tick();

 private:
  void CreateUi();
};

}  // namespace luka
