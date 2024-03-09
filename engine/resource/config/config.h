// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"
#include "core/util.h"
#include "resource/config/generated/root_path.h"

namespace luka {

class Config {
 public:
  Config();

  void Tick();

  const std::filesystem::path& GetModelPath() const;
  const std::filesystem::path& GetShaderPath() const;
  bool GetEditorMode() const;
  void SetEditorMode(bool editor_mode);

 private:
  std::filesystem::path root_path_{ReplacePathSlash(LUKA_ROOT_PATH)};
  std::filesystem::path config_path_{root_path_ / "resource" / "config" /
                                     "config.json"};
  std::filesystem::path asset_path_{root_path_ / "resource" / "asset"};

  json config_json_;

  std::filesystem::path model_path_;
  std::filesystem::path shader_path_;
  bool editor_mode_{true};
};

}  // namespace luka
