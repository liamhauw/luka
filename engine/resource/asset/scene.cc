// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene.h"

#include "core/log.h"

namespace luka {

namespace ast {

Scene::Scene(std::shared_ptr<Gpu> gpu, const cfg::Scene& cfg_scene)
    : gpu_{gpu} {
  tinygltf::Model tinygltf_model;
  std::string extension{cfg_scene.path.extension().string()};
  if (extension != ".gltf") {
    THROW("Unsupported model format.");
  }
  tinygltf::TinyGLTF tinygltf;
  std::string error;
  std::string warning;
  bool result{tinygltf.LoadASCIIFromFile(&tinygltf_model, &error, &warning,
                                         cfg_scene.path.string())};
  if (!warning.empty()) {
    LOGW("Tinygltf: {}.", warning);
  }
  if (!error.empty()) {
    LOGE("Tinygltf: {}.", error);
  }
  if (!result) {
    THROW("Fail to load {}.", cfg_scene.path.string());
  }

  map_ = sc::Map{gpu, tinygltf_model};
}

}  // namespace ast

}  // namespace luka
