// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "editor/editor_ui/editor_ui.h"

namespace luka {

EditorUi::EditorUi() {}

void EditorUi::Tick() { CreateUi(); }

void EditorUi::CreateUi() {
  bool show_demo_window{true};
  ImGui::ShowDemoWindow(&show_demo_window);
}

}  // namespace luka
