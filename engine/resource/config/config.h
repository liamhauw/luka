/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <filesystem>

#include "core/json.h"
#include "core/math.h"
#include "core/util.h"
#include "resource/config/generated/source_path.h"

namespace luka {

struct ConfigInfo {
  uint32_t scene;
  std::filesystem::path model_file_path;
  glm::vec4 camera_from;
  glm::vec4 camera_to;
};

class Config {
 public:
  Config();

  void Tick();

  const std::filesystem::path& GetModelFilePath() const;
  const glm::vec4& GetCameraFrom() const;
  const glm::vec4& GetCameraTo() const;

 private:
  std::filesystem::path source_path_{ReplacePathSlash(LUKA_SOURCE_PATH)};
  std::filesystem::path resource_path_{source_path_ / "resource"};
  std::filesystem::path config_path_{resource_path_ / "config"};
  std::filesystem::path asset_path_{resource_path_ / "asset"};
  std::filesystem::path model_path_{asset_path_ / "model"};
  std::filesystem::path shader_path_{asset_path_ /
                                     std::filesystem::path{"shader"} /
                                     std::filesystem::path{"generated"}};
  std::filesystem::path config_file_path_{config_path_ / "config.json"};

  json j_;

  ConfigInfo config_info_;
};

}  // namespace luka
