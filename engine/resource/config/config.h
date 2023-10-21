/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Config header file.
*/

#pragma once

#include <filesystem>

#include "core/util.h"
#include "resource/config/generated/source_path.h"

namespace luka {

class Config {
 public:
  Config();

  void Tick();

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

  std::filesystem::path model_file_path_;
};

}  // namespace luka
