// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/accessor.h"
#include "function/scene_graph/buffer.h"
#include "function/scene_graph/buffer_view.h"
#include "function/scene_graph/camera.h"
#include "function/scene_graph/image.h"
#include "function/scene_graph/light.h"
#include "function/scene_graph/material.h"
#include "function/scene_graph/mesh.h"
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

  std::vector<std::unique_ptr<sg::Camera>> ParseCameraComponents(
      const std::vector<tinygltf::Camera>& model_cameras);

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

  std::vector<std::unique_ptr<sg::Buffer>> ParseBufferComponents(
      const std::vector<tinygltf::Buffer>& model_buffers);

  std::vector<std::unique_ptr<sg::BufferView>> ParseBufferViewComponents(
      const std::vector<tinygltf::BufferView>& model_buffer_views,
      const std::unique_ptr<sg::Scene>& scene);

  std::vector<std::unique_ptr<sg::Accessor>> ParseAccessorComponents(
      const std::vector<tinygltf::Accessor>& model_accessors,
      const std::unique_ptr<sg::Scene>& scene);

  std::vector<std::unique_ptr<sg::Mesh>> ParseMeshComponents(
      const std::vector<tinygltf::Mesh>& model_meshs,
      const std::unique_ptr<sg::Scene>& scene);

  std::unique_ptr<sg::Light> ParseLightComponent(
      const tinygltf::Value& model_light);

  std::unique_ptr<sg::Camera> ParseCameraComponent(
      const tinygltf::Camera& model_camera);

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

  std::unique_ptr<sg::Buffer> ParseBufferComponent(
      const tinygltf::Buffer& model_buffer);

  std::unique_ptr<sg::BufferView> ParseBufferViewComponent(
      const tinygltf::BufferView& model_buffer_view,
      const std::unique_ptr<sg::Scene>& scene);

  std::unique_ptr<sg::Accessor> ParseAccessorComponent(
      const tinygltf::Accessor& model_accessor,
      const std::unique_ptr<sg::Scene>& scene);

  std::unique_ptr<sg::Mesh> ParseMeshComponent(
      const tinygltf::Mesh& model_meshs,
      const std::unique_ptr<sg::Scene>& scene,
      const vk::raii::CommandBuffer& command_buffer,
      std::vector<gpu::Buffer>& staging_buffers);

  std::shared_ptr<Gpu> gpu_;

  std::unique_ptr<sg::Scene> skybox_;
  std::unique_ptr<sg::Scene> object_;
};

}  // namespace luka
