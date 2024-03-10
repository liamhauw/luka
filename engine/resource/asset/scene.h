// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/config/config.h"

namespace luka {

namespace ast {

class Scene {
 public:
  Scene() = default;

  Scene(const cfg::Scene& cfg_scene);

 private:
  tinygltf::Model tinygltf_model_;
};

}  // namespace ast

}  // namespace luka
