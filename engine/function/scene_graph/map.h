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
#include "function/scene_graph/component.h"
#include "function/scene_graph/image.h"
#include "function/scene_graph/light.h"
#include "function/scene_graph/material.h"
#include "function/scene_graph/mesh.h"
#include "function/scene_graph/node.h"
#include "function/scene_graph/sampler.h"
#include "function/scene_graph/scene.h"
#include "function/scene_graph/texture.h"
#include "resource/asset/model.h"

namespace luka {

class Gpu;

namespace sg {

class Map {
 public:
  Map(std::shared_ptr<Gpu> gpu, const ast::Model& model,
      const std::string& name = {});

  template <typename T>
  void SetComponents(std::vector<std::unique_ptr<T>>&& components) {
    std::vector<std::unique_ptr<Component>> result(components.size());
    std::transform(
        components.begin(), components.end(), result.begin(),
        [](std::unique_ptr<T>& component) -> std::unique_ptr<Component> {
          return std::unique_ptr<Component>(std::move(component));
        });
    SetComponents(typeid(T), std::move(result));
  }

  template <typename T>
  std::vector<T*> GetComponents() const {
    std::vector<T*> result;
    if (HasComponent(typeid(T))) {
      auto& scene_components{GetComponents(typeid(T))};

      result.resize(scene_components.size());
      std::transform(scene_components.begin(), scene_components.end(),
                     result.begin(),
                     [](const std::unique_ptr<Component>& component) -> T* {
                       return dynamic_cast<T*>(component.get());
                     });
    }
    return result;
  }

  void SetComponents(const std::type_index& type_info,
                     std::vector<std::unique_ptr<Component>>&& components);

  const std::vector<std::unique_ptr<Component>>& GetComponents(
      const std::type_index& type_info) const;

  bool HasComponent(const std::type_index& type_info) const;

  void SetSupportedExtensions(
      std::unordered_map<std::string, bool>&& supported_extensions);

  const std::unordered_map<std::string, bool>& GetSupportedExtensions() const;

  void SetDefaultScene(i32 default_scene);

  void LoadScene(i32 scene = -1);

 private:
  void ParseModel(const ast::Model& model);

  void ParseExtensionsUsed(
      const std::vector<std::string>& model_extensions_used);

  void ParseLightComponents(const tinygltf::ExtensionMap& model_extension_map);

  void ParseCameraComponents(
      const std::vector<tinygltf::Camera>& model_cameras);

  void ParseImageComponents(
      const std::vector<tinygltf::Image>& tinygltf_images,
      const std::map<std::string, luka::ast::Image>& model_uri_image_map);

  void ParseSamplerComponents(
      const std::vector<tinygltf::Sampler>& model_samplers);

  void ParseTextureComponents(
      const std::vector<tinygltf::Texture>& model_textures);

  void ParseMaterialComponents(
      const std::vector<tinygltf::Material>& model_materials);

  void ParseBufferComponents(
      const std::vector<tinygltf::Buffer>& model_buffers);

  void ParseBufferViewComponents(
      const std::vector<tinygltf::BufferView>& model_buffer_views);

  void ParseAccessorComponents(
      const std::vector<tinygltf::Accessor>& model_accessors);

  void ParseMeshComponents(const std::vector<tinygltf::Mesh>& model_meshs);

  void ParseNodeComponents(const std::vector<tinygltf::Node>& model_nodes);
  void InitNodeChildren();

  void ParseSceneComponents(const std::vector<tinygltf::Scene>& model_scenes);

  void ParseDefaultScene(i32 model_scene);

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
      const tinygltf::Texture& model_texture);

  std::unique_ptr<sg::Material> ParseMaterialComponent(
      const tinygltf::Material& model_material);

  std::unique_ptr<sg::Buffer> ParseBufferComponent(
      const tinygltf::Buffer& model_buffer);

  std::unique_ptr<sg::BufferView> ParseBufferViewComponent(
      const tinygltf::BufferView& model_buffer_view);

  std::unique_ptr<sg::Accessor> ParseAccessorComponent(
      const tinygltf::Accessor& model_accessor);

  std::unique_ptr<sg::Mesh> ParseMeshComponent(
      const tinygltf::Mesh& model_mesh,
      const vk::raii::CommandBuffer& command_buffer,
      std::vector<gpu::Buffer>& staging_buffers);

  std::unique_ptr<sg::Node> ParseNodeComponent(
      const tinygltf::Node& model_node);

  std::unique_ptr<sg::Scene> ParseSceneComponent(
      const tinygltf::Scene& model_scene);

  std::shared_ptr<Gpu> gpu_;
  std::string name_;
  std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>>
      components_;
  std::unordered_map<std::string, bool> supported_extensions_;
  i32 default_scene_{-1};
};
}  // namespace sg

}  // namespace luka
