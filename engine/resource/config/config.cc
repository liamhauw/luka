/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Config source file.
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

  std::string model_name =
      ReplacePathSlash(config_data["asset"]["model"].get<std::string>());
  std::string vertex_shader_name = ReplacePathSlash(
      config_data["asset"]["shader"]["vertex"].get<std::string>());
  std::string fragment_shader_name = ReplacePathSlash(
      config_data["asset"]["shader"]["fragment"].get<std::string>());

  model_file_path_ = model_path_ / model_name;
  vertex_shader_file_path_ = shader_path_ / vertex_shader_name;
  fragment_shader_file_path_ = shader_path_ / fragment_shader_name;
}

void Config::Tick() {}

std::filesystem::path Config::GetModelFilePath() { return model_file_path_; }

std::filesystem::path Config::GetVertexShaderFilePath() {
  return vertex_shader_file_path_;
}

std::filesystem::path Config::GetFragmentShaderFilePath() {
  return fragment_shader_file_path_;
}

}  // namespace luka