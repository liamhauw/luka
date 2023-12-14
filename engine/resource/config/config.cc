// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/config/config.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Config::Config() {
  std::ifstream config_file{config_path_.string()};
  if (!config_file) {
    THROW("Fail to load config file {}", config_path_.string());
  }
  config_json_ = json::parse(config_file);
  if (config_json_.contains("config")) {
    config_ = config_json_["config"].template get<u32>();
  }
  if (config_json_.contains("configs")) {
    json configs_j{config_json_["configs"]};
    for (u32 i{0}; i < configs_j.size(); ++i) {
      json config_j{configs_j[i]};
      ConfigInfo config_info;
      if (config_j.contains("enviroment_path")) {
        config_info.enviroment_path =
            asset_path_ /
            ReplacePathSlash(
                config_j["enviroment_path"].template get<std::string>());
      }
      if (config_j.contains("skybox_path")) {
        config_info.skybox_path =
            asset_path_ /
            ReplacePathSlash(
                config_j["skybox_path"].template get<std::string>());
      }
      if (config_j.contains("object_path")) {
        config_info.object_path =
            asset_path_ /
            ReplacePathSlash(
                config_j["object_path"].template get<std::string>());
      }
      if (config_j.contains("shader_path")) {
        config_info.shader_path =
            asset_path_ /
            ReplacePathSlash(
                config_j["shader_path"].template get<std::string>());
      }
      config_infos_.push_back(std::move(config_info));
    }
  }
}

void Config::Tick() {}

const ConfigInfo& Config::GetConfigInfo() const {
  return config_infos_[config_];
}

}  // namespace luka