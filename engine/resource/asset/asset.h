// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/model.h"
#include "resource/asset/shader.h"
#include "resource/config/config.h"

namespace luka {

struct AssetInfo {
  ast::Model object;
  ast::Shader vertex;
  ast::Shader fragment;
  std::filesystem::path pipeline_cache_path;
};

class Asset {
 public:
  Asset(std::shared_ptr<Config> config);

  void Tick();

  const AssetInfo& GetAssetInfo() const;

 private:
  std::shared_ptr<Config> config_;

  AssetInfo asset_info_;
};

}  // namespace luka