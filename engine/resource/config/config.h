// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"
#include "core/util.h"
#include "resource/config/generated/root_path.h"

namespace luka {

class Config {
 public:
  Config();

  void Tick();

  const std::vector<std::filesystem::path>& GetScenePaths() const;
  const std::vector<std::filesystem::path>& GetShaderPaths() const;
  const std::vector<std::filesystem::path>& GetFrameGraphPaths() const;
  u32 GetFrameGraphIndex() const;

  bool GetEditorMode() const;
  void SetEditorMode(bool editor_mode);

 private:
  std::filesystem::path resource_path_{GetPath(LUKA_ROOT_PATH) / "resource"};
  std::filesystem::path config_path_{resource_path_ / "config" / "config.json"};
  std::filesystem::path asset_path_{resource_path_ / "asset"};
  std::filesystem::path scene_path_{asset_path_ / "scene"};
  std::filesystem::path shader_path_{asset_path_ / "shader"};
  std::filesystem::path frame_graph_path_{asset_path_ / "frame_graph"};

  json config_json_;
  std::vector<std::filesystem::path> scene_paths_;
  std::vector<std::filesystem::path> shader_paths_;
  std::vector<std::filesystem::path> frame_graph_paths_;
  u32 frame_graph_index_{0};

  bool editor_mode_{true};
};

}  // namespace luka
