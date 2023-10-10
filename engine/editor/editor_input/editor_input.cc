/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Editor input source file.
*/

#include "editor/editor_input/editor_input.h"

#include "context.h"

namespace luka {

EditorInput::EditorInput() : window_{gContext.window} {
  window_->RegisterOnKeyFunc([this](auto&& ph1, auto&& ph2, auto&& ph3,
                                    auto&& ph4) {
    OnKey(std::forward<decltype(ph1)>(ph1), std::forward<decltype(ph2)>(ph2),
          std::forward<decltype(ph3)>(ph3), std::forward<decltype(ph4)>(ph4));
  });
}

void EditorInput::Tick() {}

void EditorInput::OnKey(int key, int /*scancode*/, int action, int /*mod*/) {
  if (!gContext.is_editor_mode) {
    return;
  }

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        window_->SetWindowShouldClose();
        break;
      case GLFW_KEY_F:
        gContext.is_editor_mode = false;
        window_->focus_mode_ = false;
        break;
      default:
        break;
    }
  }
}

}  // namespace luka
