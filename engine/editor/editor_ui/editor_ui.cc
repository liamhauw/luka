// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "editor/editor_ui/editor_ui.h"

namespace luka {

EditorUi::EditorUi(std::shared_ptr<Config> config,
                   std::shared_ptr<Window> window)
    : config_{config}, window_{window} {}

void EditorUi::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (!config_->GetEditorMode()) {
    return;
  }

  CreateUi();
}

void EditorUi::CreateUi() {
  bool show_demo_window{true};
  ImGui::ShowDemoWindow(&show_demo_window);
}

}  // namespace luka
