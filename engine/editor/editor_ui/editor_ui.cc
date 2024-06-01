// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "imgui.h"
#include "platform/pch.h"
// clang-format on

#include "editor/editor_ui/editor_ui.h"

namespace luka {

EditorUi::EditorUi(std::shared_ptr<Window> window,
                   std::shared_ptr<Config> config, std::shared_ptr<Time> time)
    : window_{std::move(window)},
      config_{std::move(config)},
      time_{std::move(time)} {}

void EditorUi::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (!config_->GetGlobalContext().editor_mode) {
    return;
  }

  CreateUi();
}

void EditorUi::CreateUi() {
  ++count_;
  delta_time_sum_ += time_->GetDeltaTime();
  if (delta_time_sum_ > 1.0) {
    delta_time_ = delta_time_sum_ / count_;
    fps_ = static_cast<u64>(1.0 / delta_time_);
    count_ = 0;
    delta_time_sum_ = 0.0;
  }

  ImGui::SetNextWindowPos(ImVec2{0.0, 0.0});
  ImGui::Begin("Information", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
  ImGui::Text("Delta time: %f", delta_time_);
  ImGui::Text("FPS: %llu", fps_);

  ImGui::Text("Scenes:");
  ImGui::NewLine();
  std::unordered_map<u32, bool>& show_scenes{
      config_->GetGlobalContext().show_scenes};
  for (auto& show_scene : show_scenes) {
    ImGui::SameLine();
    bool ss{show_scene.second};
    ImGui::Checkbox(std::to_string(show_scene.first).c_str(), &ss);
    show_scene.second = ss;
  }

  ImGui::End();
}

}  // namespace luka
