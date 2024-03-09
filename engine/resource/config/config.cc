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

  u32 config{0};
  if (config_json_.contains("config")) {
    config = config_json_["config"].template get<u32>();
  }

  if (config_json_.contains("configs")) {
    json configs_j{config_json_["configs"]};
    json config_j{configs_j[config]};

    if (config_j.contains("model_path")) {
      model_path_ =
          asset_path_ /
          ReplacePathSlash(config_j["model_path"].template get<std::string>());
    }

    if (config_j.contains("shader_path")) {
      shader_path_ =
          asset_path_ /
          ReplacePathSlash(config_j["shader_path"].template get<std::string>());
    }

    if (config_j.contains("editor_mode")) {
      editor_mode_ = config_j["editor_mode"].template get<bool>();
    }
  }
}

void Config::Tick() {}

const std::filesystem::path& Config::GetModelPath() const {
  return model_path_;
}

const std::filesystem::path& Config::GetShaderPath() const {
  return shader_path_;
}

bool Config::GetEditorMode() const { return editor_mode_; }

void Config::SetEditorMode(bool editor_mode) { editor_mode_ = editor_mode; }

}  // namespace luka