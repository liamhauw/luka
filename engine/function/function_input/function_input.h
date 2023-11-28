// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Window;

enum class FunctionCommand : u32 {
  FORWARD = 1 << 0,                    // W
  BACKWARD = 1 << 1,                   // S
  LEFT = 1 << 2,                       // A
  RIGHT = 1 << 3,                      // D
  JUMP = 1 << 4,                       // Space
  SQUAT = 1 << 5,                      // Left control
  SPRINT = 1 << 6,                     // Left shift
  INVALID = static_cast<u32>(1 << 31)  // Lost focus
};

class FunctionInput {
 public:
  FunctionInput();

  void Tick();

  void OnWindowSize(i32 width, i32 height);
  void OnWindowIconify(i32 iconified);
  void OnFramebufferSize(i32 width, i32 height);
  void OnKey(i32 key, i32 scancode, i32 action, i32 mod);
  void OnCursorPos(f64 xpos, f64 ypos);

 private:
  u32 control_command_{0xFFFFFFFF};
  u32 function_command_{0};

  std::shared_ptr<Window> window_;
};

}  // namespace luka
