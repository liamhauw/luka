/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "resource/config/config.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "core/log.h"

namespace luka {

Config::Config() {
  std::ifstream config_file{config_file_path_.string()};
  if (!config_file) {
    THROW("Fail to load config file {}", config_file_path_.string());
  }
  nlohmann::json config_data{nlohmann::json::parse(config_file)};

  std::string model_name{
      ReplacePathSlash(config_data["model"].get<std::string>())};

  model_file_path_ = model_path_ / model_name;
}

void Config::Tick() {}

const std::filesystem::path& Config::GetModelFilePath() const {
  return model_file_path_;
}

}  // namespace luka