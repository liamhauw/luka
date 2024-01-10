// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/map.h"

namespace luka {

class Gpu;

class SceneGraph {
 public:
  SceneGraph();

  void Tick();

 private:
  std::shared_ptr<Gpu> gpu_;

  std::unique_ptr<sg::Map> skybox_;
  std::unique_ptr<sg::Map> object_;
};

}  // namespace luka
