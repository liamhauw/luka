// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "tiny_gltf.h"

namespace luka {

namespace ast {

class Model {
 public:
  Model() = default;
  Model(const std::filesystem::path& model_path);

  const tinygltf::Model& GetTinygltfModel() const;

 private:
  void LoadGltfModel(const std::filesystem::path& model_path);

  tinygltf::Model tinygltf_model_;
};

}  // namespace ast

}  // namespace luka
