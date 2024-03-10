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

namespace cfg {

struct Model {
  std::filesystem::path path;
};

struct Shader {
  std::filesystem::path path;
};

struct Subpass {
  std::string name;
  std::vector<u32> models;
  std::unordered_map<std::string, u32> shaders;
};

struct Pass {
  std::string name;
  std::vector<u32> subpasses;
};

struct FrameGraph {
  std::vector<u32> passes;
};

}  // namespace cfg

class Config {
 public:
  Config();

  void Tick();

  const std::vector<cfg::Model>& GetModels() const;
  const std::vector<cfg::Shader>& GetShaders() const;
  const std::vector<cfg::Subpass>& GetSubpasss() const;
  const std::vector<cfg::Pass>& GetPasss() const;
  const std::vector<cfg::FrameGraph>& GetFrameGraphs() const;
  u32 GetFrameGraph() const;

  bool GetEditorMode() const;
  void SetEditorMode(bool editor_mode);

 private:
  std::filesystem::path resource_path_{GetPath(LUKA_ROOT_PATH) / "resource"};
  std::filesystem::path config_path_{resource_path_ / "config" / "config.json"};
  std::filesystem::path asset_path_{resource_path_ / "asset"};
  std::filesystem::path model_path_{asset_path_ / "model"};
  std::filesystem::path shader_path_{asset_path_ / "shader"};

  json config_json_;
  std::vector<cfg::Model> models_;
  std::vector<cfg::Shader> shaders_;
  std::vector<cfg::Subpass> subpasses_;
  std::vector<cfg::Pass> passes_;
  std::vector<cfg::FrameGraph> frame_graphs_;
  u32 frame_graph_;

  bool editor_mode_{true};
};

}  // namespace luka
