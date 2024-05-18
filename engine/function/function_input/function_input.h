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
  kForward = 1 << 0,                    // W
  kBackward = 1 << 1,                   // S
  kLeft = 1 << 2,                       // A
  kRight = 1 << 3,                      // D
  kJump = 1 << 4,                       // Space
  kSquat = 1 << 5,                      // Left control
  kSprint = 1 << 6,                     // Left shift
  kInvalid = static_cast<u32>(1 << 31)  // Lost focus
};

class FunctionInput {
 public:
  FunctionInput(std::shared_ptr<Config> config, std::shared_ptr<Window> window);

  void Tick();

  void OnKey(i32 key, i32 scancode, i32 action, i32 mod);
  void OnCursorPos(f64 xpos, f64 ypos);

 private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<Window> window_;

  u32 control_command_{0xFFFFFFFF};
  u32 function_command_{0};
};

}  // namespace luka
