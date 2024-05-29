// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/window/window.h"
#include "resource/config/config.h"

namespace luka {

enum class FunctionCommand : u32 {
  kForward = 1U << 0U,                    // W
  kBackward = 1U << 1U,                   // S
  kLeft = 1U << 2U,                       // A
  kRight = 1U << 3U,                      // D
  kJump = 1U << 4U,                       // Space
  kSquat = 1U << 5U,                      // Left control
  kSprint = 1U << 6U,                     // Left shift
  kInvalid = static_cast<u32>(1U << 31U)  // Lost focus
};

class FunctionInput {
 public:
  FunctionInput(std::shared_ptr<Window> window, std::shared_ptr<Config> config);

  void Tick();

  void OnKey(i32 key, i32 scancode, i32 action, i32 mod);
  void OnCursorPos(f64 xpos, f64 ypos);

 private:
  std::shared_ptr<Window> window_;
  std::shared_ptr<Config> config_;

  u32 control_command_{0xFFFFFFFF};
  u32 function_command_{};
};

}  // namespace luka
