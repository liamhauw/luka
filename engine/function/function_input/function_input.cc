/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Function input source file.
*/

#include "function/function_input/function_input.h"

#include "context.h"

namespace luka {

FunctionInput::FunctionInput() {
  gContext.window->RegisterOnKeyFunc([this](auto&& ph1, auto&& ph2, auto&& ph3,
                                            auto&& ph4) {
    OnKey(std::forward<decltype(ph1)>(ph1), std::forward<decltype(ph2)>(ph2),
          std::forward<decltype(ph3)>(ph3), std::forward<decltype(ph4)>(ph4));
  });

  gContext.window->RegisterOnCursorPosFunc([this](auto&& ph1, auto&& ph2) {
    OnCursorPos(std::forward<decltype(ph1)>(ph1),
                std::forward<decltype(ph2)>(ph2));
  });
}

void FunctionInput::Tick() {}

void FunctionInput::OnKey(int key, int /*scancode*/, int action, int /*mod*/) {
  if (gContext.is_editor_mode) {
    return;
  }

  function_command_ &=
      (control_command_ ^ static_cast<unsigned>(FunctionCommand::JUMP));

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        gContext.window->SetWindowShouldClose();
        break;
      case GLFW_KEY_E:
        gContext.is_editor_mode = true;
        gContext.window->SetFocusMode(true);
        break;
      case GLFW_KEY_W:
        function_command_ |= static_cast<unsigned>(FunctionCommand::FORWARD);
        break;
      case GLFW_KEY_S:
        function_command_ |= static_cast<unsigned>(FunctionCommand::BACKWARD);
        break;
      case GLFW_KEY_A:
        function_command_ |= static_cast<unsigned>(FunctionCommand::LEFT);
        break;
      case GLFW_KEY_D:
        function_command_ |= static_cast<unsigned>(FunctionCommand::RIGHT);
        break;
      case GLFW_KEY_SPACE:
        function_command_ |= static_cast<unsigned>(FunctionCommand::JUMP);
        break;
      case GLFW_KEY_LEFT_CONTROL:
        function_command_ |= static_cast<unsigned>(FunctionCommand::SQUAT);
        break;
      case GLFW_KEY_LEFT_SHIFT:
        function_command_ |= static_cast<unsigned>(FunctionCommand::SPRINT);
        break;
      default:
        break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_W:
        function_command_ &= (control_command_ ^
                              static_cast<unsigned>(FunctionCommand::FORWARD));
        break;
      case GLFW_KEY_S:
        function_command_ &= (control_command_ ^
                              static_cast<unsigned>(FunctionCommand::BACKWARD));
        break;
      case GLFW_KEY_A:
        function_command_ &=
            (control_command_ ^ static_cast<unsigned>(FunctionCommand::LEFT));
        break;
      case GLFW_KEY_D:
        function_command_ &=
            (control_command_ ^ static_cast<unsigned>(FunctionCommand::RIGHT));
        break;
      case GLFW_KEY_LEFT_CONTROL:
        function_command_ &=
            (control_command_ ^ static_cast<unsigned>(FunctionCommand::SQUAT));
        break;
      case GLFW_KEY_LEFT_SHIFT:
        function_command_ &=
            (control_command_ ^ static_cast<unsigned>(FunctionCommand::SPRINT));
        break;
      default:
        break;
    }
  }
}
void FunctionInput::OnCursorPos(double xpos, double ypos) {
  if (gContext.window->GetFocusMode()) {
    cursor_delta_xpos_ = cursor_last_xpos_ - xpos;
    cursor_delta_ypos_ = cursor_last_ypos_ - ypos;
  }
  cursor_last_xpos_ = xpos;
  cursor_last_ypos_ = ypos;
}

}  // namespace luka
