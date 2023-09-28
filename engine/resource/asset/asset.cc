/*
  SPDX license identifier: MIT.

  Copyright (C) 2023 Liam Hauw.

  Asset source file.
*/

#include "resource/asset/asset.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <filesystem>

#include "context.h"
#include "core/log.h"

namespace luka {

Asset::Asset() {
  // Load model.
  std::filesystem::path model_file_path{gContext.config->GetModelFilePath()};

  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;
  bool loader_ret{
      loader.LoadASCIIFromFile(&model_, &err, &warn, model_file_path.string())};

  if (!warn.empty()) {
    LOGW("tinygltf load warn: [{}].", warn);
  }

  if (!err.empty()) {
    LOGE("tinygltf load error: [{}].", err);
  }

  if (!loader_ret) {
    THROW("Fail to load gltf file.");
  }
}

void Asset::Tick() {}

tinygltf::Model& Asset::GetModel() {
  return model_;
}

}  // namespace luka
