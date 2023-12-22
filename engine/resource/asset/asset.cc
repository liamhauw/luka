// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include <ctpl_stl.h>
#include <ktxvulkan.h>

#include "context.h"
#include "core/log.h"

namespace luka {

Asset::Asset() : config_{gContext.config} {
  const ConfigInfo& project_info{config_->GetConfigInfo()};
  asset_info_.skybox = LoadAssetModel(project_info.skybox_path);
  asset_info_.object = LoadAssetModel(project_info.object_path);
  asset_info_.vertext_shader_buffer =
      LoadAssetShader(project_info.shader_path / "shader.vert.spv");
  asset_info_.fragment_shader_buffer =
      LoadAssetShader(project_info.shader_path / "shader.frag.spv");
}

void Asset::Tick() {}

const AssetInfo& Asset::GetAssetInfo() const { return asset_info_; }

ast::Image Asset::LoadAssetImage(const std::filesystem::path& image_path) {
  ast::Image image;
  std::string extension{image_path.extension().string()};
  if (extension == ".ktx") {
    image = LoadKtxImage(image_path);
  } else {
    image = LoadStbImage(image_path);
  }
  return image;
}

ast::Model Asset::LoadAssetModel(const std::filesystem::path& model_path) {
  ast::Model model;
  std::string extension{model_path.extension().string()};
  if (extension == ".gltf") {
    model = LoadGltfModel(model_path);
  } else {
    THROW("Unsupported model format.");
  }
  return model;
}

std::vector<u8> Asset::LoadAssetShader(
    const std::filesystem::path& shader_path) {
  return LoadBinary(shader_path);
}

ast::Image Asset::LoadKtxImage(const std::filesystem::path& image_path) {
  std::vector<u8> binary_data{LoadBinary(image_path)};

  ktxTexture* texture;
  u8* buffer{binary_data.data()};
  u64 size{binary_data.size()};

  auto result{ktxTexture_CreateFromMemory(
      buffer, size, KTX_TEXTURE_CREATE_NO_FLAGS, &texture)};

  if (result != KTX_SUCCESS) {
    THROW("Fail to create ktx texture.");
  }

  return ast::Image{};
}

ast::Image Asset::LoadStbImage(const std::filesystem::path& image_path) {
  return ast::Image{};
}

ast::Model Asset::LoadGltfModel(const std::filesystem::path& model_path) {
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

  std::map<std::string, ast::Image> url_image_map;

  const std::vector<tinygltf::Image>& images{tinygltf_model.images};
  u64 image_count{images.size()};

  // // Multithread.
  // u32 thread_count{std::thread::hardware_concurrency()};
  // thread_count = thread_count == 0 ? 1 : thread_count;
  // ctpl::thread_pool thread_pool{static_cast<i32>(thread_count)};

  // std::vector<std::future<std::pair<std::string, ast::Image>>>
  //     future_url_image_map;

  // for (u64 i{0}; i < image_count; ++i) {
  //   const std::string& image_url{images[i].uri};
  //   auto future_image{thread_pool.push([&model_path, &image_url, this](u64) {
  //     std::filesystem::path image_path{model_path.parent_path() / image_url};
  //     auto image{LoadAssetImage(image_path)};
  //     return std::make_pair(image_url, image);
  //   })};
  //   future_url_image_map.push_back(std::move(future_image));
  // }

  // for (u64 i{0}; i < image_count; ++i) {
  //   url_image_map.insert(future_url_image_map[i].get());
  // }

  for (u64 i{0}; i < image_count; ++i) {
    const std::string& image_url{images[i].uri};
    std::filesystem::path image_path{model_path.parent_path() / image_url};
    auto image{LoadAssetImage(image_path)};
    url_image_map.insert(std::make_pair(image_url, image));
  }

  ast::Model model{std::move(tinygltf_model), std::move(url_image_map)};

  return model;
}

std::vector<u8> Asset::LoadBinary(const std::filesystem::path& binary_path) {
  std::vector<u8> binary_data;

  std::ifstream binary_file(binary_path.string(),
                            std::ios::ate | std::ios::binary);
  if (!binary_file) {
    THROW("Fail to open {}", binary_path.string());
  }

  u32 size{static_cast<u32>(binary_file.tellg())};
  binary_data.resize(size);
  binary_file.seekg(0);
  binary_file.read(reinterpret_cast<char*>(binary_data.data()), size);
  binary_file.close();

  return binary_data;
}

}  // namespace luka
