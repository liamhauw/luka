// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/asset/scene_component/map.h"
#include "resource/config/config.h"
#include "resource/gpu/gpu.h"

namespace luka {

namespace ast {

class Scene {
 public:
  Scene() = default;

  Scene(std::shared_ptr<Gpu> gpu, const cfg::Scene& cfg_scene);

 private:
  std::shared_ptr<Gpu> gpu_;
  sc::Map map_;
};

}  // namespace ast

}  // namespace luka
