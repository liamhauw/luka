// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/image.h"
#include "resource/asset/model.h"
#include "resource/config/config.h"

namespace luka {

struct AssetInfo {
  ast::Model object;
  std::vector<u8> vertext_shader_buffer;
  std::vector<u8> fragment_shader_buffer;
};

class Asset {
 public:
  Asset(std::shared_ptr<Config> config);

  void Tick();

  const AssetInfo& GetAssetInfo() const;

 private:
  ast::Image LoadAssetImage(const std::filesystem::path& image_path);
  ast::Model LoadAssetModel(const std::filesystem::path& model_path);
  std::vector<u8> LoadAssetShader(const std::filesystem::path& shader_path);

  ast::Image LoadKtxImage(const std::filesystem::path& image_path);
  ast::Image LoadStbImage(const std::filesystem::path& image_path);
  ast::Model LoadGltfModel(const std::filesystem::path& model_path);
  std::vector<u8> LoadBinary(const std::filesystem::path& binary_path);

  std::shared_ptr<Config> config_;

  AssetInfo asset_info_;
};

}  // namespace luka