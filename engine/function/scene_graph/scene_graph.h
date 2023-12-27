// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/image.h"
#include "function/scene_graph/light.h"
#include "function/scene_graph/sampler.h"
#include "function/scene_graph/scene.h"
#include "function/scene_graph/texture.h"

namespace luka {

namespace ast {
class Model;
}  // namespace ast

class Gpu;

class SceneGraph {
 public:
  SceneGraph();

  void Tick();

 private:
  std::unique_ptr<sg::Scene> LoadScene(const ast::Model& model);

  std::unordered_map<std::string, bool> ParseExtensionsUsed(
      const ast::Model& model);
  std::vector<std::unique_ptr<sg::Light>> ParseLightComponents(
      const ast::Model& model,
      const std::unordered_map<std::string, bool>& supported_extensions);
  std::vector<std::unique_ptr<sg::Image>> ParseImageComponents(
      const ast::Model& model);
  std::vector<std::unique_ptr<sg::Sampler>> ParseSamplerComponents(
      const ast::Model& model);
  std::vector<std::unique_ptr<sg::Texture>> ParseTextureComponents(
      const ast::Model& model, const std::unique_ptr<sg::Scene>& scene);

  std::shared_ptr<Gpu> gpu_;

  std::unique_ptr<sg::Scene> skybox_;
  std::unique_ptr<sg::Scene> object_;
};

}  // namespace luka
