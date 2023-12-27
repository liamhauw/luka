// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/model.h"

namespace luka {

namespace ast {

Model::Model(tinygltf::Model&& tinygltf_model,
             std::map<std::string, Image>&& uri_texture_map)
    : tinygltf_model_{std::move(tinygltf_model)},
      uri_image_map_{std::move(uri_texture_map)} {}

const tinygltf::Model& Model::GetTinygltfModel() const {
  return tinygltf_model_;
}

const std::map<std::string, Image>& Model::GetUriTextureMap() const {
  return uri_image_map_;
}

}  // namespace ast

}  // namespace luka
