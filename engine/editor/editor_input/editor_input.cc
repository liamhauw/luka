// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "editor/editor_input/editor_input.h"

#include "core/log.h"

namespace luka {

EditorInput::EditorInput(std::shared_ptr<Config> config,
                         std::shared_ptr<Window> window,
                         std::shared_ptr<Time> time,
                         std::shared_ptr<Camera> camera)
    : config_{config}, window_{window}, time_{time}, camera_{camera} {
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

void EditorInput::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  f64 delta_time{time_->GetDeltaTime()};
  f32 velocity{2.0F};

  f32 move_distance{static_cast<f32>(delta_time * velocity)};

  glm::vec3 camera_relative_pos{0.0F};
  bool has_move{false};
  if (editor_command_ & static_cast<u32>(EditorCommand::kFoward)) {
    camera_relative_pos += glm::vec3{0.0F, 0.0F, -move_distance};
    has_move = true;
  }
  if (editor_command_ & static_cast<u32>(EditorCommand::kBackward)) {
    camera_relative_pos += glm::vec3{0.0F, 0.0F, move_distance};
    has_move = true;
  }
  if (editor_command_ & static_cast<u32>(EditorCommand::kLeft)) {
    camera_relative_pos += glm::vec3{-move_distance, 0.0F, 0.0F};
    has_move = true;
  }
  if (editor_command_ & static_cast<u32>(EditorCommand::kRight)) {
    camera_relative_pos += glm::vec3{move_distance, 0.0F, 0.0F};
    has_move = true;
  }
  if (editor_command_ & static_cast<u32>(EditorCommand::kUp)) {
    camera_relative_pos += glm::vec3{0.0F, move_distance, 0.0F};
    has_move = true;
  }
  if (editor_command_ & static_cast<u32>(EditorCommand::kDown)) {
    camera_relative_pos += glm::vec3{0.0F, -move_distance, 0.0F};
    has_move = true;
  }

  if (has_move) {
    camera_->Move(camera_relative_pos);
  }
}

void EditorInput::OnKey(i32 key, i32 /*scancode*/, i32 action, i32 /*mod*/) {
  if (!config_->GetEditorMode()) {
    return;
  }

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        window_->SetWindowShouldClose();
        break;
      case GLFW_KEY_1:
        LOGI("Change to function mode");
        config_->SetEditorMode(false);
        break;
      case GLFW_KEY_W:
        editor_command_ |= static_cast<u32>(EditorCommand::kFoward);
        break;
      case GLFW_KEY_S:
        editor_command_ |= static_cast<u32>(EditorCommand::kBackward);
        break;
      case GLFW_KEY_A:
        editor_command_ |= static_cast<u32>(EditorCommand::kLeft);
        break;
      case GLFW_KEY_D:
        editor_command_ |= static_cast<u32>(EditorCommand::kRight);
        break;
      case GLFW_KEY_Q:
        editor_command_ |= static_cast<u32>(EditorCommand::kUp);
        break;
      case GLFW_KEY_E:
        editor_command_ |= static_cast<u32>(EditorCommand::kDown);
        break;
      default:
        break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_W:
        editor_command_ &=
            (editor_command_ ^ static_cast<u32>(EditorCommand::kFoward));
        break;
      case GLFW_KEY_S:
        editor_command_ &=
            (editor_command_ ^ static_cast<u32>(EditorCommand::kBackward));
        break;
      case GLFW_KEY_A:
        editor_command_ &=
            (editor_command_ ^ static_cast<u32>(EditorCommand::kLeft));
        break;
      case GLFW_KEY_D:
        editor_command_ &=
            (editor_command_ ^ static_cast<u32>(EditorCommand::kRight));
        break;
      case GLFW_KEY_Q:
        editor_command_ &=
            (editor_command_ ^ static_cast<u32>(EditorCommand::kUp));
        break;
      case GLFW_KEY_E:
        editor_command_ &=
            (editor_command_ ^ static_cast<u32>(EditorCommand::kDown));
        break;
      default:
        break;
    }
  }
}

void EditorInput::OnCursorPos(f64 xpos, f64 ypos) {
  if (!config_->GetEditorMode()) {
    return;
  }

  if (window_->IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT)) {
    i32 window_width{0};
    i32 window_height{0};
    window_->GetWindowSize(&window_width, &window_height);

    f32 angular_velocity{180.0F / std::max(window_width, window_height)};
    f32 yaw{
        glm::radians(static_cast<f32>(prev_xpos_ - xpos) * angular_velocity)};
    f32 pitch{
        glm::radians(static_cast<f32>(prev_ypos_ - ypos) * angular_velocity)};

    camera_->Rotate(yaw, pitch);
  }

  prev_xpos_ = xpos;
  prev_ypos_ = ypos;
}

}  // namespace luka
