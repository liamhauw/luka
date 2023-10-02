/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Function input header file.
*/

#pragma once

namespace luka {

enum class FunctionCommand : unsigned {
  FORWARD = 1 << 0,                         // W
  BACKWARD = 1 << 1,                        // S
  LEFT = 1 << 2,                            // A
  RIGHT = 1 << 3,                           // D
  JUMP = 1 << 4,                            // SPACE
  SQUAT = 1 << 5,                           // LEFT CONTROL
  SPRINT = 1 << 6,                          // LEFT SHIFT
  INVALID = static_cast<unsigned>(1 << 31)  // lost focus
};

class FunctionInput {
 public:
  FunctionInput();

  void Tick();

  void OnKey(int key, int scancode, int action, int mod);
  void OnCursorPos(double xpos, double ypos);

 private:
  unsigned control_command_{0xFFFFFFFF};

  unsigned function_command_{0};

  double cursor_last_xpos_{0.0};
  double cursor_last_ypos_{0.0};

  double cursor_delta_xpos_{0.0};
  double cursor_delta_ypos_{0.0};
};

}  // namespace luka
