// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include <ctpl_stl.h>

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

  std::map<std::string, ast::Image> url_texture_map;

  const std::vector<tinygltf::Image>& images{tinygltf_model.images};
  u64 image_count{images.size()};

  u32 thread_count{std::thread::hardware_concurrency()};
  thread_count = thread_count == 0 ? 1 : thread_count;
  ctpl::thread_pool thread_pool{static_cast<i32>(thread_count)};

  std::vector<std::future<std::unique_ptr<ast::Image>>> future_images;

  for (u64 i{0}; i < image_count; ++i) {
    auto future{thread_pool.push([&images, i](u64) {
      LOGI("{}", images[i].uri);
      return std::make_unique<ast::Image>();
    })};
    future_images.push_back(std::move(future));
  }

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
