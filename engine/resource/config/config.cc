// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/config/config.h"

#include "context.h"
#include "core/log.h"

namespace luka {

void from_json(const json& j, ConfigInfo& config_info) {
  if (j.contains("scenes")) {
    const json& scene_json{j.at("scenes").at(config_info.scene)};
    if (scene_json.contains("model")) {
      scene_json.at("model").get_to(config_info.model_file_path);
    }
    if (scene_json.contains("camera") &&
        scene_json.at("camera").contains("from") &&
        scene_json.at("camera").contains("to")) {
      scene_json.at("camera").at("from").get_to(config_info.camera_from);
      scene_json.at("camera").at("to").get_to(config_info.camera_to);
    }
  }
}

Config::Config() {
  std::ifstream config_file{config_file_path_.string()};
  if (!config_file) {
    THROW("Fail to load config file {}", config_file_path_.string());
  }
  j_ = json::parse(config_file);
  if (j_.contains("scene")) {
    j_.at("scene").get_to(config_info_.scene);
  }
}

void Config::Tick() {
  if (gContext.load) {
    j_.get_to(config_info_);
    config_info_.model_file_path = model_path_ / config_info_.model_file_path;
  }
}

const std::filesystem::path& Config::GetModelFilePath() const {
  return config_info_.model_file_path;
}

const glm::vec4& Config::GetCameraFrom() const {
  return config_info_.camera_from;
}

const glm::vec4& Config::GetCameraTo() const { return config_info_.camera_to; }

}  // namespace luka