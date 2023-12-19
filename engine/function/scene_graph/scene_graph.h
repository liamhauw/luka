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

  vk::Filter ParseMagFilter(int mag_filter);
  vk::Filter ParseMinFilter(int min_filter);
  vk::SamplerMipmapMode ParseMipmapMode(int min_filter);
  vk::SamplerAddressMode ParseAddressMode(int wrap);

  std::unique_ptr<sg::Scene> skybox_;
  std::unique_ptr<sg::Scene> object_;
};

}  // namespace luka
