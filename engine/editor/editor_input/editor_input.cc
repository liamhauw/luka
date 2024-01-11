// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "editor/editor_input/editor_input.h"

namespace luka {

EditorInput::EditorInput(std::shared_ptr<Context> context,
                         std::shared_ptr<Window> window)
    : context_{context}, window_{window} {
  window_->RegisterOnKeyFunc([this](auto&& ph1, auto&& ph2, auto&& ph3,
                                    auto&& ph4) {
    OnKey(std::forward<decltype(ph1)>(ph1), std::forward<decltype(ph2)>(ph2),
          std::forward<decltype(ph3)>(ph3), std::forward<decltype(ph4)>(ph4));
  });
}

void EditorInput::Tick() {}

void EditorInput::OnKey(i32 key, i32 /*scancode*/, i32 action, i32 /*mod*/) {
  if (!context_->GetEditorMode()) {
    return;
  }

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        window_->SetWindowShouldClose();
        break;
      case GLFW_KEY_F:
        context_->SetEditorMode(false);
        window_->SetFocusMode(false);
        break;
      default:
        break;
    }
  }
}

}  // namespace luka
