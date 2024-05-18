// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene.h"

#include "core/log.h"

namespace luka::ast {

Scene::Scene(std::shared_ptr<Gpu> gpu,
             const std::filesystem::path& cfg_scene_path,
             const vk::raii::CommandBuffer& command_buffer,
             std::vector<gpu::Buffer>& staging_buffers)
    : gpu_{std::move(gpu)} {
  tinygltf::Model tinygltf;
  std::string extension{cfg_scene_path.extension().string()};
  if (extension != ".gltf") {
    THROW("Unsupported tinygltf format.");
  }
  tinygltf::TinyGLTF tg;
  std::string error;
  std::string warning;
  bool result{tg.LoadASCIIFromFile(&tinygltf, &error, &warning,
                                   cfg_scene_path.string())};
  if (!warning.empty()) {
    LOGW("Tinygltf: {}.", warning);
  }
  if (!error.empty()) {
    LOGE("Tinygltf: {}.", error);
  }
  if (!result) {
    THROW("Fail to load {}.", cfg_scene_path.string());
  }

  ParseExtensionsUsed(tinygltf.extensionsUsed);
  ParseLightComponents(tinygltf.extensions);
  ParseCameraComponents(tinygltf.cameras);
  ParseImageComponents(tinygltf.images, command_buffer, staging_buffers);
  ParseSamplerComponents(tinygltf.samplers);
  ParseTextureComponents(tinygltf.textures);
  ParseMaterialComponents(tinygltf.materials);
  ParseBufferComponents(tinygltf.buffers);
  ParseBufferViewComponents(tinygltf.bufferViews);
  ParseAccessorComponents(tinygltf.accessors);
  ParseMeshComponents(tinygltf.meshes, command_buffer, staging_buffers);
  ParseNodeComponents(tinygltf.nodes);
  ParseSceneComponents(tinygltf.scenes);
  ParseDefaultScene(tinygltf.defaultScene);
}

Scene::Scene(Scene&& rhs) noexcept
    : gpu_{std::move(rhs.gpu_)},
      name_{std::exchange(rhs.name_, {})},
      components_{std::exchange(rhs.components_, {})},
      supported_extensions_{std::exchange(rhs.supported_extensions_, {})},
      scene_{rhs.scene_} {}

Scene& Scene::operator=(Scene&& rhs) noexcept {
  if (this != &rhs) {
    gpu_ = rhs.gpu_;
    std::swap(name_, rhs.name_);
    std::swap(components_, rhs.components_);
    std::swap(supported_extensions_, rhs.supported_extensions_);
    scene_ = rhs.scene_;
  }
  return *this;
}

void Scene::AddComponent(const std::type_index& type_info,
                         std::unique_ptr<sc::Component>&& component) {
  components_[type_info].push_back(std::move(component));
}

const std::vector<std::unique_ptr<sc::Component>>& Scene::GetComponents(
    const std::type_index& type_info) const {
  return components_.at(type_info);
}

bool Scene::HasComponent(const std::type_index& type_info) const {
  auto iter{components_.find(type_info)};
  return (iter != components_.end() && !iter->second.empty());
}

void Scene::SetSupportedExtensions(
    std::unordered_map<std::string, bool>&& supported_extensions) {
  supported_extensions_ = std::move(supported_extensions);
}

const std::unordered_map<std::string, bool>& Scene::GetSupportedExtensions()
    const {
  return supported_extensions_;
}

void Scene::LoadScene(i32 scene) {
  auto scene_components{GetComponents<sc::Scene>()};
  i32 scene_count{static_cast<i32>(scene_components.size())};
  if (scene >= 0 && scene < scene_count) {
    scene_ = scene;
  }

  auto node_components{GetComponents<sc::Node>()};
  for (sc::Node* node : node_components) {
    node->ClearParent();
  }

  sc::Scene* cur_scene{scene_components[scene_]};
  const std::vector<sc::Node*>& nodes{cur_scene->GetNodes()};

  sc::Node* root_node{nullptr};
  std::queue<std::pair<sc::Node*, sc::Node*>> traverse_nodes;
  for (sc::Node* node : nodes) {
    traverse_nodes.emplace(root_node, node);
  }

  while (!traverse_nodes.empty()) {
    auto iter{traverse_nodes.front()};
    traverse_nodes.pop();

    sc::Node* parent{iter.first};
    sc::Node* child{iter.second};

    child->SetParent(parent);

    const std::vector<sc::Node*>& child_children{child->GetChildren()};
    for (sc::Node* child_child : child_children) {
      traverse_nodes.emplace(child, child_child);
    }
  }
}

const sc::Scene* Scene::GetScene() const {
  auto scene_components{GetComponents<sc::Scene>()};
  return scene_components[scene_];
}

void Scene::ParseExtensionsUsed(
    const std::vector<std::string>& tinygltf_extensions_used) {
  std::unordered_map<std::string, bool> supported_extensions{
      {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

  for (const std::string& tinygltf_extension_used : tinygltf_extensions_used) {
    auto iter = supported_extensions.find(tinygltf_extension_used);
    if (iter == supported_extensions.end()) {
      LOGW("Contain unsupported extension : {}", tinygltf_extension_used);
    } else {
      iter->second = true;
    }
  }

  SetSupportedExtensions(std::move(supported_extensions));
}

void Scene::ParseLightComponents(
    const tinygltf::ExtensionMap& tinygltf_extension_map) {
  const std::unordered_map<std::string, bool>& supported_extensions{
      GetSupportedExtensions()};

  auto khr_lights_punctual_extension{
      supported_extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION)};
  if (khr_lights_punctual_extension != supported_extensions.end() &&
      khr_lights_punctual_extension->second) {
    if (tinygltf_extension_map.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) !=
            tinygltf_extension_map.end() &&
        tinygltf_extension_map.at(KHR_LIGHTS_PUNCTUAL_EXTENSION)
            .Has("lights")) {
      const auto& tinygltf_lights{
          tinygltf_extension_map.at(KHR_LIGHTS_PUNCTUAL_EXTENSION)
              .Get("lights")};

      for (u64 i{0}; i < tinygltf_lights.ArrayLen(); ++i) {
        auto light_component{std::make_unique<sc::Light>(
            tinygltf_lights.Get(static_cast<i32>(i)))};
        AddComponent(std::move(light_component));
      }
    }
  }
}

void Scene::ParseCameraComponents(
    const std::vector<tinygltf::Camera>& tinygltf_cameras) {
  for (const auto& tinygltf_camera : tinygltf_cameras) {
    auto camera_component{std::make_unique<sc::Camera>(tinygltf_camera)};
    AddComponent(std::move(camera_component));
  }
}

void Scene::ParseImageComponents(
    const std::vector<tinygltf::Image>& tinygltf_images,
    const vk::raii::CommandBuffer& command_buffer,
    std::vector<gpu::Buffer>& staging_buffers) {
  u64 tinygltf_image_count{tinygltf_images.size()};

  for (u64 i{0}; i < tinygltf_image_count; ++i) {
    const tinygltf::Image& tinygltf_image{tinygltf_images[i]};

    auto image_component{std::make_unique<sc::Image>(
        gpu_, tinygltf_image, command_buffer, staging_buffers)};
    AddComponent(std::move(image_component));
  }

  tinygltf::Image default_tinygltf_image;
  default_tinygltf_image.name = "default";
  default_tinygltf_image.width = 1;
  default_tinygltf_image.height = 1;
  default_tinygltf_image.component = 4;
  default_tinygltf_image.bits = 8;
  default_tinygltf_image.image = std::vector<u8>(4, 0);

  auto default_image_component{std::make_unique<sc::Image>(
      gpu_, default_tinygltf_image, command_buffer, staging_buffers)};
  AddComponent(std::move(default_image_component));
}

void Scene::ParseSamplerComponents(
    const std::vector<tinygltf::Sampler>& tinygltf_samplers) {
  for (const auto& tinygltf_sampler : tinygltf_samplers) {
    auto sampler_component{
        std::make_unique<sc::Sampler>(gpu_, tinygltf_sampler)};
    AddComponent(std::move(sampler_component));
  }

  tinygltf::Sampler default_tinygltf_sampler;
  default_tinygltf_sampler.name = "default";
  default_tinygltf_sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
  default_tinygltf_sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
  default_tinygltf_sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
  default_tinygltf_sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
  auto default_sampler_component{
      std::make_unique<sc::Sampler>(gpu_, default_tinygltf_sampler)};
  AddComponent(std::move(default_sampler_component));
}

void Scene::ParseTextureComponents(
    const std::vector<tinygltf::Texture>& tinygltf_textures) {
  auto image_components{GetComponents<sc::Image>()};
  auto sampler_components{GetComponents<sc::Sampler>()};

  for (const auto& tinygltf_texture : tinygltf_textures) {
    auto texture_component(std::make_unique<sc::Texture>(
        image_components, sampler_components, tinygltf_texture));
    AddComponent(std::move(texture_component));
  }

  tinygltf::Texture default_tinygltf_texture;
  default_tinygltf_texture.name = "default";

  auto default_texture_component{std::make_unique<sc::Texture>(
      image_components, sampler_components, default_tinygltf_texture)};
  AddComponent(std::move(default_texture_component));
}

void Scene::ParseMaterialComponents(
    const std::vector<tinygltf::Material>& tinygltf_materials) {
  auto texture_components{GetComponents<sc::Texture>()};

  for (const auto& tinygltf_material : tinygltf_materials) {
    auto material_component{
        std::make_unique<sc::Material>(texture_components, tinygltf_material)};
    AddComponent(std::move(material_component));
  }

  tinygltf::Material default_tinygltf_material;
  default_tinygltf_material.name = "default";
  auto default_material_component{std::make_unique<sc::Material>(
      texture_components, default_tinygltf_material)};
  AddComponent(std::move(default_material_component));
}

void Scene::ParseBufferComponents(
    const std::vector<tinygltf::Buffer>& tinygltf_buffers) {
  for (const auto& tinygltf_buffer : tinygltf_buffers) {
    std::unique_ptr<sc::Buffer> buffer_component{
        std::make_unique<sc::Buffer>(tinygltf_buffer)};
    AddComponent(std::move(buffer_component));
  }
}

void Scene::ParseBufferViewComponents(
    const std::vector<tinygltf::BufferView>& tinygltf_buffer_views) {
  auto buffer_components{GetComponents<sc::Buffer>()};

  for (const auto& tinygltf_buffer_view : tinygltf_buffer_views) {
    std::unique_ptr<sc::BufferView> buffer_view_component{
        std::make_unique<sc::BufferView>(buffer_components,
                                         tinygltf_buffer_view)};
    AddComponent(std::move(buffer_view_component));
  }
}

void Scene::ParseAccessorComponents(
    const std::vector<tinygltf::Accessor>& tinygltf_accessors) {
  auto buffer_view_components{GetComponents<sc::BufferView>()};

  for (const auto& tinygltf_accessor : tinygltf_accessors) {
    std::unique_ptr<sc::Accessor> accessor_component{
        std::make_unique<sc::Accessor>(buffer_view_components,
                                       tinygltf_accessor)};
    AddComponent(std::move(accessor_component));
  }
}

void Scene::ParseMeshComponents(
    const std::vector<tinygltf::Mesh>& tinygltf_meshs,
    const vk::raii::CommandBuffer& command_buffer,
    std::vector<gpu::Buffer>& staging_buffers) {
  auto material_components{GetComponents<sc::Material>()};
  auto accessor_components{GetComponents<sc::Accessor>()};

  for (const auto& tinygltf_mesh : tinygltf_meshs) {
    std::unique_ptr<sc::Mesh> mesh_component{std::make_unique<sc::Mesh>(
        gpu_, material_components, accessor_components, tinygltf_mesh,
        command_buffer, staging_buffers)};
    AddComponent(std::move(mesh_component));
  }
}

void Scene::ParseNodeComponents(
    const std::vector<tinygltf::Node>& tinygltf_nodes) {
  auto light_components{GetComponents<sc::Light>()};
  auto camera_components{GetComponents<sc::Camera>()};
  auto mesh_components{GetComponents<sc::Mesh>()};

  for (const auto& tinygltf_node : tinygltf_nodes) {
    std::unique_ptr<sc::Node> node_component{std::make_unique<sc::Node>(
        light_components, camera_components, mesh_components, tinygltf_node)};
    AddComponent(std::move(node_component));
  }
  InitNodeChildren();
}

void Scene::InitNodeChildren() const {
  auto node_components{GetComponents<sc::Node>()};

  for (sc::Node* node : node_components) {
    const std::vector<i32>& child_indices{node->GetChildIndices()};
    std::vector<sc::Node*> children_node;
    children_node.reserve(child_indices.size());
    for (i32 child_index : child_indices) {
      children_node.push_back(node_components[child_index]);
    }
    node->SetChildren(std::move(children_node));
  }
}

void Scene::ParseSceneComponents(
    const std::vector<tinygltf::Scene>& tinygltf_scenes) {
  auto node_components{GetComponents<sc::Node>()};

  for (const auto& tinygltf_scene : tinygltf_scenes) {
    std::unique_ptr<sc::Scene> scene_component{
        std::make_unique<sc::Scene>(node_components, tinygltf_scene)};
    AddComponent(std::move(scene_component));
  }
}

void Scene::ParseDefaultScene(i32 tinygltf_default_scene) {
  i32 scene_count{static_cast<i32>(GetComponents<sc::Scene>().size())};
  if (tinygltf_default_scene != -1 && tinygltf_default_scene < scene_count) {
    scene_ = tinygltf_default_scene;
  } else if (scene_count > 0) {
    scene_ = 0;
  } else {
    LOGW("gltf doesn't have scene.");
  }
}

}  // namespace luka::ast
