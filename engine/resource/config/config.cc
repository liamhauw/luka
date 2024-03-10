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
    const json& scene_jsons{config_json_["scenes"]};
    for (const json& scene_json : scene_jsons) {
      std::string uri{scene_json.template get<std::string>()};
      std::filesystem::path path{scene_path_ / GetPath(uri)};
      scenes_.emplace_back(path);
    }
  }
  if (config_json_.contains("shaders")) {
    const json& shdaer_jsons{config_json_["shaders"]};
    for (const json& shader_json : shdaer_jsons) {
      std::string uri{shader_json.template get<std::string>()};
      std::filesystem::path path{shader_path_ / GetPath(uri)};
      shaders_.emplace_back(path);
    }
  }

  if (config_json_.contains("subpasses")) {
    const json& subpass_jsons{config_json_["subpasses"]};
    for (const json& subpass_json : subpass_jsons) {
      cfg::Subpass subpass;
      if (subpass_json.contains("name")) {
        subpass.name = subpass_json["name"].template get<std::string>();
      }
      if (subpass_json.contains("scenes")) {
        const json& scene_jsons{subpass_json["scenes"]};
        for (const auto& scene_json : scene_jsons) {
          u32 scene{scene_json.template get<u32>()};
          subpass.scenes.push_back(scene);
        }
      }
      if (subpass_json.contains("shaders")) {
        const json& shader_jsons{subpass_json["shaders"]};
        if (shader_jsons.contains("vertex")) {
          u32 shader{shader_jsons["vertex"].template get<u32>()};
          subpass.shaders.emplace("vertex", shader);
        }
        if (shader_jsons.contains("fragment")) {
          u32 shader{shader_jsons["fragment"].template get<u32>()};
          subpass.shaders.emplace("fragment", shader);
        }
      }
      subpasses_.push_back(std::move(subpass));
    }
  }

  if (config_json_.contains("passes")) {
    const json& pass_jsons{config_json_["passes"]};
    for (const auto& pass_json : pass_jsons) {
      cfg::Pass pass;
      if (pass_json.contains("name")) {
        pass.name = pass_json["name"].template get<std::string>();
      }
      if (pass_json.contains("subpasses")) {
        const json& subpass_jsons{pass_json["subpasses"]};
        for (const json& subpass_json : subpass_jsons) {
          u32 subpass{subpass_json.template get<u32>()};
          pass.subpasses.push_back(subpass);
        }
      }
      passes_.push_back(pass);
    }
  }

  if (config_json_.contains("frame_graph")) {
    const json& frame_graph_jsons{config_json_["frame_graph"]};
    for (const auto& frame_graph_json : frame_graph_jsons) {
      cfg::FrameGraph frame_graph;
      if (frame_graph_json.contains("passes")) {
        const json& pass_jsons{frame_graph_json["passes"]};
        for (const json& pass_json : pass_jsons) {
          u32 pass{pass_json.template get<u32>()};
          frame_graph.passes.push_back(pass);
        }
      }
      frame_graphs_.push_back(std::move(frame_graph));
    }
  }

  if (config_json_.contains("frame_graph")) {
    frame_graph_ = config_json_["frame_graph"].template get<u32>();
  }
}

void Config::Tick() {}

const std::vector<cfg::Scene>& Config::GetScenes() const { return scenes_; }

const std::vector<cfg::Shader>& Config::GetShaders() const { return shaders_; }

const std::vector<cfg::Subpass>& Config::GetSubpasss() const {
  return subpasses_;
}

const std::vector<cfg::Pass>& Config::GetPasss() const { return passes_; }

const std::vector<cfg::FrameGraph>& Config::GetFrameGraphs() const {
  return frame_graphs_;
}

u32 Config::GetFrameGraph() const { return frame_graph_; }

bool Config::GetEditorMode() const { return editor_mode_; }

void Config::SetEditorMode(bool editor_mode) { editor_mode_ = editor_mode; }

}  // namespace luka