// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/map.h"

#include "core/log.h"
#include "core/util.h"

namespace luka {

namespace sg {

Map::Map(std::shared_ptr<Gpu> gpu, const ast::Model& model,
         const std::string& name)
    : gpu_{gpu}, name_{name} {
  const tinygltf::Model& tinygltf_model{model.GetTinygltfModel()};
  const std::map<std::string, ast::Image>& uri_image_map{
      model.GetUriImageMap()};

  ParseExtensionsUsed(tinygltf_model.extensionsUsed);
  ParseLightComponents(tinygltf_model.extensions);
  ParseCameraComponents(tinygltf_model.cameras);
  ParseImageComponents(tinygltf_model.images, uri_image_map);
  ParseSamplerComponents(tinygltf_model.samplers);
  ParseTextureComponents(tinygltf_model.textures);
  ParseMaterialComponents(tinygltf_model.materials);
  ParseBufferComponents(tinygltf_model.buffers);
  ParseBufferViewComponents(tinygltf_model.bufferViews);
  ParseAccessorComponents(tinygltf_model.accessors);
  ParseMeshComponents(tinygltf_model.meshes);
  ParseNodeComponents(tinygltf_model.nodes);
  ParseSceneComponents(tinygltf_model.scenes);
  ParseDefaultScene(tinygltf_model.defaultScene);
}

void Map::AddComponent(const std::type_index& type_info,
                       std::unique_ptr<Component>&& component) {
  components_[type_info].push_back(std::move(component));
}

const std::vector<std::unique_ptr<Component>>& Map::GetComponents(
    const std::type_index& type_info) const {
  return components_.at(type_info);
}

bool Map::HasComponent(const std::type_index& type_info) const {
  auto iter{components_.find(type_info)};
  return (iter != components_.end() && !iter->second.empty());
}

void Map::SetSupportedExtensions(
    std::unordered_map<std::string, bool>&& supported_extensions) {
  supported_extensions_ = std::move(supported_extensions);
}

const std::unordered_map<std::string, bool>& Map::GetSupportedExtensions()
    const {
  return supported_extensions_;
}

void Map::LoadScene(i32 scene) {
  auto scene_components{GetComponents<Scene>()};
  i32 scene_count{static_cast<i32>(scene_components.size())};
  if (scene >= 0 && scene < scene_count) {
    scene_ = scene;
  }

  auto node_components{GetComponents<Node>()};
  for (Node* node : node_components) {
    node->ClearParent();
  }

  Scene* cur_scene{scene_components[scene_]};
  const std::vector<Node*>& nodes{cur_scene->GetNodes()};

  Node* root_node{nullptr};
  std::queue<std::pair<Node*, Node*>> traverse_nodes;
  for (Node* node : nodes) {
    traverse_nodes.push(std::make_pair(root_node, node));
  }

  while (!traverse_nodes.empty()) {
    auto iter{traverse_nodes.front()};
    traverse_nodes.pop();

    Node* parent{iter.first};
    Node* child{iter.second};

    child->SetParent(parent);

    const std::vector<Node*> child_children{child->GetChildren()};
    for (Node* child_child : child_children) {
      traverse_nodes.push(std::make_pair(child, child_child));
    }
  }
}

const sg::Scene* Map::GetScene() const {
  auto scene_components{GetComponents<sg::Scene>()};
  return scene_components[scene_];
}

void Map::ParseExtensionsUsed(
    const std::vector<std::string>& model_extensions_used) {
  std::unordered_map<std::string, bool> supported_extensions{
      {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

  for (const std::string& model_extension_used : model_extensions_used) {
    auto iter = supported_extensions.find(model_extension_used);
    if (iter == supported_extensions.end()) {
      LOGW("Contain unsupported extension : {}", model_extension_used);
    } else {
      iter->second = true;
    }
  }

  SetSupportedExtensions(std::move(supported_extensions));
}

void Map::ParseLightComponents(
    const tinygltf::ExtensionMap& model_extension_map) {
  const std::unordered_map<std::string, bool>& supported_extensions{
      GetSupportedExtensions()};

  auto khr_lights_punctual_extension{
      supported_extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION)};
  if (khr_lights_punctual_extension != supported_extensions.end() &&
      khr_lights_punctual_extension->second) {
    if (model_extension_map.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) !=
            model_extension_map.end() &&
        model_extension_map.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Has("lights")) {
      const auto& model_lights{
          model_extension_map.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Get("lights")};

      for (u64 i{0}; i < model_lights.ArrayLen(); ++i) {
        auto light_component{
            std::make_unique<Light>(model_lights.Get(static_cast<i32>(i)))};
        AddComponent(std::move(light_component));
      }
    }
  }
}

void Map::ParseCameraComponents(
    const std::vector<tinygltf::Camera>& model_cameras) {
  for (const auto& model_camera : model_cameras) {
    auto camera_component{std::make_unique<Camera>(model_camera)};
    AddComponent(std::move(camera_component));
  }
}

void Map::ParseImageComponents(
    const std::vector<tinygltf::Image>& tinygltf_images,
    const std::map<std::string, ast::Image>& uri_image_map) {
  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginTempCommandBuffer()};

  u64 model_image_count{tinygltf_images.size()};

  std::vector<gpu::Buffer> staging_buffers;

  for (u64 i{0}; i < model_image_count; ++i) {
    const tinygltf::Image& tinygltf_image{tinygltf_images[i]};

    const std::string& model_image_uri{tinygltf_image.uri};
    auto u2i{uri_image_map.find(model_image_uri)};
    if (u2i == uri_image_map.end()) {
      THROW("Fail to find image uri");
    }
    const ast::Image& model_image{u2i->second};
    auto image_component{std::make_unique<Image>(
        gpu_, model_image, command_buffer, staging_buffers)};
    AddComponent(std::move(image_component));
  }

  ast::Image default_model_image{std::vector<u8>(4, 0),
                                 {vk::Extent3D{1, 1, 1}},
                                 vk::Format::eR8G8B8A8Unorm};
  auto default_image_component{std::make_unique<Image>(
      gpu_, default_model_image, command_buffer, staging_buffers)};
  AddComponent(std::move(default_image_component));

  gpu_->EndTempCommandBuffer(command_buffer);
}

void Map::ParseSamplerComponents(
    const std::vector<tinygltf::Sampler>& model_samplers) {
  for (const auto& model_sampler : model_samplers) {
    auto sampler_component{std::make_unique<Sampler>(gpu_, model_sampler)};
    AddComponent(std::move(sampler_component));
  }

  tinygltf::Sampler default_model_sampler;
  default_model_sampler.name = "default";
  default_model_sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
  default_model_sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
  default_model_sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
  default_model_sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
  auto default_sampler_component{
      std::make_unique<Sampler>(gpu_, default_model_sampler)};
  AddComponent(std::move(default_sampler_component));
}

void Map::ParseTextureComponents(
    const std::vector<tinygltf::Texture>& model_textures) {
  auto image_components{GetComponents<Image>()};
  auto sampler_components{GetComponents<Sampler>()};

  for (const auto& model_texture : model_textures) {
    auto texture_component(std::make_unique<Texture>(
        image_components, sampler_components, model_texture));
    AddComponent(std::move(texture_component));
  }

  tinygltf::Texture default_model_texture;
  default_model_texture.name = "default";

  auto default_texture_component{std::make_unique<Texture>(
      image_components, sampler_components, default_model_texture)};
  AddComponent(std::move(default_texture_component));
}

void Map::ParseMaterialComponents(
    const std::vector<tinygltf::Material>& model_materials) {
  auto texture_components{GetComponents<Texture>()};

  for (const auto& model_material : model_materials) {
    auto material_component{
        std::make_unique<Material>(texture_components, model_material)};
    AddComponent(std::move(material_component));
  }

  tinygltf::Material default_model_material;
  default_model_material.name = "default";
  auto default_material_component{
      std::make_unique<Material>(texture_components, default_model_material)};
  AddComponent(std::move(default_material_component));
}

void Map::ParseBufferComponents(
    const std::vector<tinygltf::Buffer>& model_buffers) {
  for (const auto& model_buffer : model_buffers) {
    std::unique_ptr<Buffer> buffer_component{
        std::make_unique<Buffer>(model_buffer)};
    AddComponent(std::move(buffer_component));
  }
}

void Map::ParseBufferViewComponents(
    const std::vector<tinygltf::BufferView>& model_buffer_views) {
  auto buffer_components{GetComponents<Buffer>()};

  for (const auto& model_buffer_view : model_buffer_views) {
    std::unique_ptr<BufferView> buffer_view_component{
        std::make_unique<BufferView>(buffer_components, model_buffer_view)};
    AddComponent(std::move(buffer_view_component));
  }
}

void Map::ParseAccessorComponents(
    const std::vector<tinygltf::Accessor>& model_accessors) {
  auto buffer_view_components{GetComponents<BufferView>()};

  for (const auto& model_accessor : model_accessors) {
    std::unique_ptr<Accessor> accessor_component{
        std::make_unique<Accessor>(buffer_view_components, model_accessor)};
    AddComponent(std::move(accessor_component));
  }
}

void Map::ParseMeshComponents(const std::vector<tinygltf::Mesh>& model_meshs) {
  auto material_components{GetComponents<Material>()};
  auto accessor_components{GetComponents<Accessor>()};

  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginTempCommandBuffer()};

  std::vector<gpu::Buffer> staging_buffers;

  for (const auto& model_mesh : model_meshs) {
    std::unique_ptr<Mesh> mesh_component{
        std::make_unique<Mesh>(gpu_, material_components, accessor_components,
                               model_mesh, command_buffer, staging_buffers)};
    AddComponent(std::move(mesh_component));
  }

  gpu_->EndTempCommandBuffer(command_buffer);
}

void Map::ParseNodeComponents(const std::vector<tinygltf::Node>& model_nodes) {
  auto light_components{GetComponents<Light>()};
  auto camera_components{GetComponents<Camera>()};
  auto mesh_components{GetComponents<Mesh>()};

  for (const auto& model_node : model_nodes) {
    std::unique_ptr<Node> node_component{std::make_unique<Node>(
        light_components, camera_components, mesh_components, model_node)};
    AddComponent(std::move(node_component));
  }
  InitNodeChildren();
}

void Map::InitNodeChildren() {
  auto node_components{GetComponents<Node>()};

  for (Node* node : node_components) {
    const std::vector<i32>& child_indices{node->GetChildIndices()};
    std::vector<Node*> children_node;
    for (i32 child_index : child_indices) {
      children_node.push_back(node_components[child_index]);
    }
    node->SetChildren(std::move(children_node));
  }
}

void Map::ParseSceneComponents(
    const std::vector<tinygltf::Scene>& model_scenes) {
  auto node_components{GetComponents<Node>()};

  for (const auto& model_scene : model_scenes) {
    std::unique_ptr<Scene> scene_component{
        std::make_unique<Scene>(node_components, model_scene)};
    AddComponent(std::move(scene_component));
  }
}

void Map::ParseDefaultScene(i32 model_default_scene) {
  i32 scene_count{static_cast<i32>(GetComponents<Scene>().size())};
  if (model_default_scene != -1 && model_default_scene < scene_count) {
    scene_ = model_default_scene;
  } else if (scene_count > 0) {
    scene_ = 0;
  } else {
    LOGW("gltf doesn't have scene.");
  }
}

}  // namespace sg

}  // namespace luka
