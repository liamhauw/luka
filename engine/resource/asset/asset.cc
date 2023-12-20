// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Asset::Asset() : config_{gContext.config} {
  const ConfigInfo& project_info{config_->GetConfigInfo()};
  asset_info_.skybox = LoadModel(project_info.skybox_path);
  asset_info_.object = LoadModel(project_info.object_path);
  asset_info_.vertext_shader_buffer =
      LoadShader(project_info.shader_path / "shader.vert.spv");
  asset_info_.fragment_shader_buffer =
      LoadShader(project_info.shader_path / "shader.frag.spv");
}

void Asset::Tick() {}

const AssetInfo& Asset::GetAssetInfo() const { return asset_info_; }

ast::Model Asset::LoadModel(const std::filesystem::path& model_path) {
  tinygltf::Model tinygltf_model;
  tinygltf::TinyGLTF tinygltf;
  std::string error;
  std::string warning;

  bool result{tinygltf.LoadASCIIFromFile(&tinygltf_model, &error, &warning,
                                         model_path.string())};
  if (!warning.empty()) {
    LOGW("Tinygltf: {}.", warning);
  }
  if (!error.empty()) {
    LOGE("Tinygltf: {}.", error);
  }
  if (!result) {
    THROW("Fail to load {}.", model_path.string());
  }

  std::map<std::string, ast::Texture> url_texture_map;

  ast::Model model{std::move(tinygltf_model), std::move(url_texture_map)};

  return model;
}

std::vector<u8> Asset::LoadShader(const std::filesystem::path& shader_path) {
  std::vector<u8> shader_buffer;
  std::ifstream shader_file(shader_path.string(),
                            std::ios::ate | std::ios::binary);
  if (!shader_file) {
    THROW("Fail to open {}", shader_path.string());
  }
  u32 file_size{static_cast<u32>(shader_file.tellg())};
  shader_buffer.resize(file_size);
  shader_file.seekg(0);
  shader_file.read(reinterpret_cast<char*>(shader_buffer.data()), file_size);
  shader_file.close();
  return shader_buffer;
}

}  // namespace luka
