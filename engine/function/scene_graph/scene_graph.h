// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/image.h"
#include "function/scene_graph/light.h"
#include "function/scene_graph/material.h"
#include "function/scene_graph/sampler.h"
#include "function/scene_graph/scene.h"
#include "function/scene_graph/texture.h"
#include "resource/asset/model.h"

namespace luka {

namespace ast {
class Model;
}  // namespace ast

namespace gpu {
class Buffer;
}

class Gpu;

class SceneGraph {
 public:
  SceneGraph();

  void Tick();

 private:
  std::unique_ptr<sg::Scene> LoadScene(const ast::Model& model);

  std::unordered_map<std::string, bool> ParseExtensionsUsed(
      const std::vector<std::string>& model_extensions_used);

  std::vector<std::unique_ptr<sg::Light>> ParseLightComponents(
      const tinygltf::ExtensionMap& model_extension_map,
      const std::unordered_map<std::string, bool>& supported_extensions);

  std::vector<std::unique_ptr<sg::Image>> ParseImageComponents(
      const std::vector<tinygltf::Image>& tinygltf_images,
      const std::map<std::string, luka::ast::Image>& model_uri_image_map);

  std::vector<std::unique_ptr<sg::Sampler>> ParseSamplerComponents(
      const std::vector<tinygltf::Sampler>& model_samplers);

  std::vector<std::unique_ptr<sg::Texture>> ParseTextureComponents(
      const std::vector<tinygltf::Texture>& model_textures,
      const std::unique_ptr<sg::Scene>& scene);

  std::vector<std::unique_ptr<sg::Material>> ParseMaterialComponents(
      const std::vector<tinygltf::Material>& model_materials,
      const std::unique_ptr<sg::Scene>& scene);

  std::unique_ptr<sg::Light> ParseLightComponent(
      const tinygltf::Value& model_light);

  std::unique_ptr<sg::Image> ParseImageComponent(
      const ast::Image& model_image,
      const vk::raii::CommandBuffer& command_buffer,
      std::vector<gpu::Buffer>& staging_buffers);

  std::unique_ptr<sg::Sampler> ParseSamplerComponent(
      const tinygltf::Sampler& model_sampler);

  std::unique_ptr<sg::Texture> ParseTextureComponent(
      const tinygltf::Texture& model_texture,
      const std::unique_ptr<sg::Scene>& scene);

  std::unique_ptr<sg::Material> ParseMaterialComponent(
      const tinygltf::Material& model_material,
      const std::unique_ptr<sg::Scene>& scene);

  std::shared_ptr<Gpu> gpu_;

  std::unique_ptr<sg::Scene> skybox_;
  std::unique_ptr<sg::Scene> object_;
};

}  // namespace luka
