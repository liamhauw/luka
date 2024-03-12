// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/asset/scene_component/accessor.h"
#include "resource/asset/scene_component/buffer.h"
#include "resource/asset/scene_component/buffer_view.h"
#include "resource/asset/scene_component/camera.h"
#include "resource/asset/scene_component/component.h"
#include "resource/asset/scene_component/image.h"
#include "resource/asset/scene_component/light.h"
#include "resource/asset/scene_component/material.h"
#include "resource/asset/scene_component/mesh.h"
#include "resource/asset/scene_component/node.h"
#include "resource/asset/scene_component/sampler.h"
#include "resource/asset/scene_component/scene.h"
#include "resource/asset/scene_component/texture.h"
#include "resource/config/config.h"
#include "resource/gpu/gpu.h"

namespace luka {

namespace ast {

class Scene {
 public:
  Scene() = default;

  Scene(std::shared_ptr<Gpu> gpu, const cfg::Scene& cfg_scene);

  Scene(Scene&& rhs);

  Scene& operator=(Scene&& rhs);

  template <typename T>
  void AddComponent(std::unique_ptr<T>&& component) {
    std::unique_ptr<sc::Component> result{std::move(component)};
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
                     [](const std::unique_ptr<sc::Component>& component) -> T* {
                       return dynamic_cast<T*>(component.get());
                     });
    }
    return result;
  }

  void AddComponent(const std::type_index& type_info,
                    std::unique_ptr<sc::Component>&& component);

  const std::vector<std::unique_ptr<sc::Component>>& GetComponents(
      const std::type_index& type_info) const;

  bool HasComponent(const std::type_index& type_info) const;

  void SetSupportedExtensions(
      std::unordered_map<std::string, bool>&& supported_extensions);

  const std::unordered_map<std::string, bool>& GetSupportedExtensions() const;

  void LoadScene(i32 scene = -1);

  const ast::sc::Scene* GetScene() const;

 private:
  void ParseExtensionsUsed(
      const std::vector<std::string>& tinygltf_extensions_used);

  void ParseLightComponents(
      const tinygltf::ExtensionMap& tinygltf_extension_map);

  void ParseCameraComponents(
      const std::vector<tinygltf::Camera>& tinygltf_cameras);

  void ParseImageComponents(
      const std::vector<tinygltf::Image>& tinygltf_images);

  void ParseSamplerComponents(
      const std::vector<tinygltf::Sampler>& tinygltf_samplers);

  void ParseTextureComponents(
      const std::vector<tinygltf::Texture>& tinygltf_textures);

  void ParseMaterialComponents(
      const std::vector<tinygltf::Material>& tinygltf_materials);

  void ParseBufferComponents(
      const std::vector<tinygltf::Buffer>& tinygltf_buffers);

  void ParseBufferViewComponents(
      const std::vector<tinygltf::BufferView>& tinygltf_buffer_views);

  void ParseAccessorComponents(
      const std::vector<tinygltf::Accessor>& tinygltf_accessors);

  void ParseMeshComponents(const std::vector<tinygltf::Mesh>& tinygltf_meshs);

  void ParseNodeComponents(const std::vector<tinygltf::Node>& tinygltf_nodes);
  void InitNodeChildren();

  void ParseSceneComponents(
      const std::vector<tinygltf::Scene>& tinygltf_scenes);

  void ParseDefaultScene(i32 tinygltf_default_scene);

  std::shared_ptr<Gpu> gpu_;

  std::string name_;
  std::unordered_map<std::type_index,
                     std::vector<std::unique_ptr<sc::Component>>>
      components_;
  std::unordered_map<std::string, bool> supported_extensions_;
  i32 scene_;
};

}  // namespace ast

}  // namespace luka
