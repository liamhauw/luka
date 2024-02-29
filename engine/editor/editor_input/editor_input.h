// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"
#include "function/context/context.h"
#include "function/time/time.h"
#include "function/window/window.h"

namespace luka {

enum class EditorCommand : u32 {
  kFoward = 1 << 0,    // W
  kBackward = 1 << 1,  // S
  kLeft = 1 << 2,      // A
  kRight = 1 << 3,     // D
  kUp = 1 << 4,        // Q
  kDown = 1 << 5       // E
};

class EditorInput {
 public:
  EditorInput(std::shared_ptr<Context> context, std::shared_ptr<Time> time,
              std::shared_ptr<Window> window, std::shared_ptr<Camera> camera);

  void Tick();

  void OnKey(i32 key, i32 scancode, i32 action, i32 mod);
  void OnCursorPos(f64 xpos, f64 ypos);
  void OnCursorEnter(int entered);

 private:
  std::shared_ptr<Context> context_;
  std::shared_ptr<Time> time_;
  std::shared_ptr<Window> window_;
  std::shared_ptr<Camera> camera_;

  f32 cursor_xpos_{0};
  f32 cursor_ypos_{0};

  u32 editor_command_{0};
};

}  // namespace luka
