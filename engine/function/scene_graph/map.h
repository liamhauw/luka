// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
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
#include "resource/asset/image.h"
#include "resource/asset/model.h"

namespace luka {

namespace sg {

class Map {
 public:
  Map() = default;

  Map(std::shared_ptr<Gpu> gpu, const ast::Model& model,
      const std::string& name = {});

  template <typename T>
  void AddComponent(std::unique_ptr<T>&& component) {
    std::unique_ptr<Component> result{std::move(component)};
    AddComponent(typeid(T), std::move(result));
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

  void AddComponent(const std::type_index& type_info,
                    std::unique_ptr<Component>&& component);

  const std::vector<std::unique_ptr<Component>>& GetComponents(
      const std::type_index& type_info) const;

  bool HasComponent(const std::type_index& type_info) const;

  void SetSupportedExtensions(
      std::unordered_map<std::string, bool>&& supported_extensions);

  const std::unordered_map<std::string, bool>& GetSupportedExtensions() const;

  void SetDefaultScene(i32 default_scene);

  void LoadScene(i32 scene = -1);

 private:
  void ParseExtensionsUsed(
      const std::vector<std::string>& model_extensions_used);

  void ParseLightComponents(const tinygltf::ExtensionMap& model_extension_map);

  void ParseCameraComponents(
      const std::vector<tinygltf::Camera>& model_cameras);

  void ParseImageComponents(const std::vector<tinygltf::Image>& tinygltf_images,
                            const std::map<std::string, ast::Image>&);

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

  std::shared_ptr<Gpu> gpu_;
  std::string name_;
  std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>>
      components_;
  std::unordered_map<std::string, bool> supported_extensions_;
  i32 default_scene_;
};
}  // namespace sg

}  // namespace luka
