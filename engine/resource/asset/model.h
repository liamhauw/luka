// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/image.h"
#include <tiny_gltf.h>

namespace luka {

namespace ast {

class Model {
 public:
  Model() = default;
  
  Model(const std::filesystem::path& model_path);

  const tinygltf::Model& GetTinygltfModel() const;
  const std::map<std::string, Image>& GetUriImageMap() const;

 private:
  tinygltf::Model tinygltf_model_;
  std::map<std::string, Image> uri_image_map_;
};

}  // namespace ast

}  // namespace luka
