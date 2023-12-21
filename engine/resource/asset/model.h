// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/image.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

namespace luka {

namespace ast {

class Model {
 public:
  Model() = default;
  Model(tinygltf::Model&& tinygltf_model,
        std::map<std::string, Image>&& url_texture_map);

  const tinygltf::Model& GetTinygltfModel() const;
  const std::map<std::string, Image>& GetUrlTextureMap() const;

 private:
  tinygltf::Model tinygltf_model_;
  std::map<std::string, Image> url_texture_map_;
};

}  // namespace ast

}  // namespace luka
