// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/model.h"

#include "core/log.h"

namespace luka {

namespace ast {

Model::Model(const cfg::Model& cfg_model) {
  std::string extension{cfg_model.path.extension().string()};
  if (extension != ".gltf") {
    THROW("Unsupported model format.");
  }
  tinygltf::TinyGLTF tinygltf;
  std::string error;
  std::string warning;
  bool result{tinygltf.LoadASCIIFromFile(&tinygltf_model_, &error, &warning,
                                         cfg_model.path.string())};
  if (!warning.empty()) {
    LOGW("Tinygltf: {}.", warning);
  }
  if (!error.empty()) {
    LOGE("Tinygltf: {}.", error);
  }
  if (!result) {
    THROW("Fail to load {}.", cfg_model.path.string());
  }

  std::vector<tinygltf::Image>& tinygltf_images{tinygltf_model_.images};
  for (tinygltf::Image& tinygltf_image : tinygltf_images) {
    const std::string& uri{tinygltf_image.uri};
    ast::Image image{tinygltf_image};
    uri_image_map_.insert(std::make_pair(uri, image));
  }
}

const tinygltf::Model& Model::GetTinygltfModel() const {
  return tinygltf_model_;
}

const std::map<std::string, Image>& Model::GetUriImageMap() const {
  return uri_image_map_;
}

}  // namespace ast

}  // namespace luka
