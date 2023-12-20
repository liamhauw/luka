// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/model.h"
#include "resource/asset/texture.h"
#include "resource/config/config.h"

namespace luka {

struct AssetInfo {
  ast::Model skybox;
  ast::Model object;
  std::vector<u8> vertext_shader_buffer;
  std::vector<u8> fragment_shader_buffer;
};

class Asset {
 public:
  Asset();

  void Tick();

  const AssetInfo& GetAssetInfo() const;

 private:
  ast::Model LoadModel(const std::filesystem::path& model_path);
  std::vector<u8> LoadShader(const std::filesystem::path& shader_path);

  std::shared_ptr<Config> config_;

  AssetInfo asset_info_;
};

}  // namespace luka