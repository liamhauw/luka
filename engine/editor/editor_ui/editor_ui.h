// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include "function/time/time.h"
#include "resource/config/config.h"
#include "resource/window/window.h"

namespace luka {

class EditorUi {
 public:
  EditorUi(std::shared_ptr<Config> config, std::shared_ptr<Window> window,
           std::shared_ptr<Time> time);

  void Tick();

 private:
  void CreateUi();

  std::shared_ptr<Config> config_;
  std::shared_ptr<Window> window_;
  std::shared_ptr<Time> time_;

  u32 count_ = 0;
  f64 delta_time_sum_ = 0.0;

  f64 delta_time_ = 0.0;
  u64 fps_ = 0;
};

}  // namespace luka
