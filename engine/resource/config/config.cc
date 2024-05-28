// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/config/config.h"

#include "core/log.h"

namespace luka {

Config::Config() {
  std::ifstream config_file{config_path_.string()};
  if (!config_file) {
    THROW("Fail to load config file {}", config_path_.string());
  }
  config_json_ = json::parse(config_file);

  if (config_json_.contains("scenes")) {
    const json& scenes_json{config_json_["scenes"]};
    for (const json& scene_json : scenes_json) {
      std::string uri{scene_json.template get<std::string>()};
      scene_paths_.push_back(scene_path_ / GetPath(uri));
    }
  }

  if (config_json_.contains("lights")) {
    const json& lights_json{config_json_["lights"]};
    for (const json& light_json : lights_json) {
      std::string uri{light_json.template get<std::string>()};
      light_paths_.push_back(light_path_ / GetPath(uri));
    }
  }

  if (config_json_.contains("shaders")) {
    const json& shaders_json{config_json_["shaders"]};
    for (const json& shader_json : shaders_json) {
      std::string uri{shader_json.template get<std::string>()};
      shader_paths_.push_back(shader_path_ / GetPath(uri));
    }
  }

  if (config_json_.contains("frame_graphs")) {
    const json& frame_graphs_json{config_json_["frame_graphs"]};
    for (const json& frame_graph_json : frame_graphs_json) {
      std::string uri{frame_graph_json.template get<std::string>()};
      frame_graph_paths_.push_back(frame_graph_path_ / GetPath(uri));
    }
  }

  if (config_json_.contains("frame_graph")) {
    frame_graph_index_ = config_json_["frame_graph"].template get<u32>();
  }
}

void Config::Tick() {}

GlobalContext& Config::GetGlobalContext() { return global_context_; }

const std::vector<std::filesystem::path>& Config::GetScenePaths() const {
  return scene_paths_;
}

const std::vector<std::filesystem::path>& Config::GetLightPaths() const {
  return light_paths_;
}

const std::vector<std::filesystem::path>& Config::GetShaderPaths() const {
  return shader_paths_;
}

const std::vector<std::filesystem::path>& Config::GetFrameGraphPaths() const {
  return frame_graph_paths_;
}

u32 Config::GetFrameGraphIndex() const { return frame_graph_index_; }

}  // namespace luka