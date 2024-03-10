// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/asset/image.h"
#include "resource/config/config.h"

namespace luka {

namespace ast {

class Model {
 public:
  Model() = default;

  Model(const cfg::Model& cfg_model);

  const tinygltf::Model& GetTinygltfModel() const;
  const std::map<std::string, Image>& GetUriImageMap() const;

 private:
  tinygltf::Model tinygltf_model_;
  std::map<std::string, Image> uri_image_map_;
};

}  // namespace ast

}  // namespace luka
