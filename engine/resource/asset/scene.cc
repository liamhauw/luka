// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene.h"

#include "core/log.h"

namespace luka {

namespace ast {

Scene::Scene(const cfg::Scene& cfg_scene) {
  std::string extension{cfg_scene.path.extension().string()};
  if (extension != ".gltf") {
    THROW("Unsupported model format.");
  }
  tinygltf::TinyGLTF tinygltf;
  std::string error;
  std::string warning;
  bool result{tinygltf.LoadASCIIFromFile(&tinygltf_model_, &error, &warning,
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
}

}  // namespace ast

}  // namespace luka
