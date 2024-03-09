// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include "core/log.h"

namespace luka {

Asset::Asset(std::shared_ptr<Config> config) : config_{config} {
  asset_info_.object = std::move(ast::Model{config_->GetModelPath()});

  asset_info_.vertex = std::move(
      ast::Shader{config_->GetShaderPath() / "shader.vert", EShLangVertex});
  asset_info_.fragment = std::move(
      ast::Shader{config_->GetShaderPath() / "shader.frag", EShLangFragment});
}

void Asset::Tick() {}

const AssetInfo& Asset::GetAssetInfo() const { return asset_info_; }

}  // namespace luka
