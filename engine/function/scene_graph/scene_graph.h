// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene.h"

namespace tinygltf {
class Model;
}

namespace luka {

class SceneGraph {
 public:
  SceneGraph();

  void Tick();

 private:
  std::unique_ptr<sg::Scene> LoadScene(const tinygltf::Model& model);

  std::unique_ptr<sg::Scene> skybox_;
  std::unique_ptr<sg::Scene> object_;
};

}  // namespace luka
