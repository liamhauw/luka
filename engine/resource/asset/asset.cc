// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include "core/log.h"

namespace luka {

Asset::Asset(std::shared_ptr<Config> config) : config_{config} {
  const ConfigInfo& config_info{config_->GetConfigInfo()};

  asset_info_.object = std::move(ast::Model{config_info.object_path});

  asset_info_.vertex = std::move(
      ast::Shader{config_info.shader_path / "shader.vert", EShLangVertex});
  asset_info_.fragment = std::move(
      ast::Shader{config_info.shader_path / "shader.frag", EShLangFragment});

  asset_info_.pipeline_cache_path =
      config_->GetAssetPath() / "generated" / "pipeline_cache";
}

void Asset::Tick() {}

const AssetInfo& Asset::GetAssetInfo() const { return asset_info_; }

}  // namespace luka
