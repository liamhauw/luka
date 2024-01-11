// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/model.h"

#include "core/log.h"

namespace luka {

namespace ast {

Model::Model(const std::filesystem::path& model_path) {
  std::string extension{model_path.extension().string()};
  if (extension == ".gltf") {
    LoadGltfModel(model_path);
  } else {
    THROW("Unsupported model format.");
  }
}

const tinygltf::Model& Model::GetTinygltfModel() const {
  return tinygltf_model_;
}

void Model::LoadGltfModel(const std::filesystem::path& model_path) {
  tinygltf::TinyGLTF tinygltf;
  std::string error;
  std::string warning;
  bool result{tinygltf.LoadASCIIFromFile(&tinygltf_model_, &error, &warning,
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
}

}  // namespace ast

}  // namespace luka
