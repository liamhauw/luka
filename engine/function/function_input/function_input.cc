// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/function_input/function_input.h"

#include "core/log.h"

namespace luka {

FunctionInput::FunctionInput(std::shared_ptr<Config> config,
                             std::shared_ptr<Window> window)
    : config_{std::move(config)}, window_{std::move(window)} {
  window_->RegisterOnKeyFunc([this](auto&& ph1, auto&& ph2, auto&& ph3,
                                    auto&& ph4) {
    OnKey(std::forward<decltype(ph1)>(ph1), std::forward<decltype(ph2)>(ph2),
          std::forward<decltype(ph3)>(ph3), std::forward<decltype(ph4)>(ph4));
  });

  window_->RegisterOnCursorPosFunc([this](auto&& ph1, auto&& ph2) {
    OnCursorPos(std::forward<decltype(ph1)>(ph1),
                std::forward<decltype(ph2)>(ph2));
  });
}

void FunctionInput::Tick() {}

void FunctionInput::OnKey(i32 key, i32 /*scancode*/, i32 action, i32 /*mods*/) {
  if (config_->GetGlobalContext().editor_mode) {
    return;
  }

  function_command_ &=
      (control_command_ ^ static_cast<u32>(FunctionCommand::kJump));

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        window_->SetWindowShouldClose();
        break;
      case GLFW_KEY_2:
        LOGI("Change to eidtor mode");
        config_->GetGlobalContext().editor_mode = true;
        break;
      case GLFW_KEY_W:
        function_command_ |= static_cast<u32>(FunctionCommand::kForward);
        break;
      case GLFW_KEY_S:
        function_command_ |= static_cast<u32>(FunctionCommand::kBackward);
        break;
      case GLFW_KEY_A:
        function_command_ |= static_cast<u32>(FunctionCommand::kLeft);
        break;
      case GLFW_KEY_D:
        function_command_ |= static_cast<u32>(FunctionCommand::kRight);
        break;
      case GLFW_KEY_SPACE:
        function_command_ |= static_cast<u32>(FunctionCommand::kJump);
        break;
      case GLFW_KEY_LEFT_CONTROL:
        function_command_ |= static_cast<u32>(FunctionCommand::kSquat);
        break;
      case GLFW_KEY_LEFT_SHIFT:
        function_command_ |= static_cast<u32>(FunctionCommand::kSprint);
        break;
      default:
        break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_W:
        function_command_ &=
            (control_command_ ^ static_cast<u32>(FunctionCommand::kForward));
        break;
      case GLFW_KEY_S:
        function_command_ &=
            (control_command_ ^ static_cast<u32>(FunctionCommand::kBackward));
        break;
      case GLFW_KEY_A:
        function_command_ &=
            (control_command_ ^ static_cast<u32>(FunctionCommand::kLeft));
        break;
      case GLFW_KEY_D:
        function_command_ &=
            (control_command_ ^ static_cast<u32>(FunctionCommand::kRight));
        break;
      case GLFW_KEY_LEFT_CONTROL:
        function_command_ &=
            (control_command_ ^ static_cast<u32>(FunctionCommand::kSquat));
        break;
      case GLFW_KEY_LEFT_SHIFT:
        function_command_ &=
            (control_command_ ^ static_cast<u32>(FunctionCommand::kSprint));
        break;
      default:
        break;
    }
  }
}

void FunctionInput::OnCursorPos(f64 /*xpos*/, f64 /*ypos*/) {
  if (config_->GetGlobalContext().editor_mode) {
    return;
  }
}

}  // namespace luka
