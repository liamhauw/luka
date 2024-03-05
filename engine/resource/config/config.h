// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"
#include "core/util.h"
#include "resource/config/generated/source_path.h"

namespace luka {

struct ConfigInfo {
  std::filesystem::path object_path;
  std::filesystem::path shader_path;
};

class Config {
 public:
  Config();

  void Tick();

  const std::filesystem::path GetAssetPath() const;
  const ConfigInfo& GetConfigInfo() const;

 private:
  std::filesystem::path source_path_{ReplacePathSlash(LUKA_SOURCE_PATH)};
  std::filesystem::path config_path_{source_path_ / "resource" / "config" /
                                     "config.json"};
  std::filesystem::path asset_path_{source_path_ / "resource" / "asset"};

  json config_json_;
  u32 config_;
  std::vector<ConfigInfo> config_infos_;
};

}  // namespace luka
