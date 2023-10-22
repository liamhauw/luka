/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <memory>

namespace luka {

class Window;

enum class FunctionCommand : unsigned {
  FORWARD = 1 << 0,                         // W
  BACKWARD = 1 << 1,                        // S
  LEFT = 1 << 2,                            // A
  RIGHT = 1 << 3,                           // D
  JUMP = 1 << 4,                            // Sace
  SQUAT = 1 << 5,                           // Left control
  SPRINT = 1 << 6,                          // Left shift
  INVALID = static_cast<unsigned>(1 << 31)  // Lost focus
};

class FunctionInput {
 public:
  FunctionInput();

  void Tick();

  void OnWindowSize(int width, int height);
  void OnWindowIconify(int iconified);
  void OnFramebufferSize(int width, int height);
  void OnKey(int key, int scancode, int action, int mod);
  void OnCursorPos(double xpos, double ypos);

 private:
  unsigned control_command_{0xFFFFFFFF};
  unsigned function_command_{0};

  std::shared_ptr<Window> window_;
};

}  // namespace luka
