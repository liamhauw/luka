// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene_graph.h"

#include <ctpl_stl.h>

#include "context.h"
#include "core/log.h"
#include "core/util.h"

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace luka {

SceneGraph::SceneGraph() : gpu_{gContext.gpu} {
  const AssetInfo& asset_info{gContext.asset->GetAssetInfo()};
  const ast::Model& skybox{asset_info.skybox};
  const ast::Model& object{asset_info.object};

  // skybox_ = LoadScene(skybox);
  object_ = LoadScene(object);
}

void SceneGraph::Tick() {}

std::unique_ptr<sg::Scene> SceneGraph::LoadScene(const ast::Model& model) {
  auto scene{std::make_unique<sg::Scene>("gltf_scene")};

  const tinygltf::Model& tinygltf_model{model.GetTinygltfModel()};

  std::unordered_map<std::string, bool> supported_extensions{
      ParseExtensionsUsed(tinygltf_model.extensionsUsed)};

  std::vector<std::unique_ptr<sg::Light>> light_components{
      ParseLightComponents(tinygltf_model.extensions, supported_extensions)};
  scene->SetComponents(std::move(light_components));

  std::vector<std::unique_ptr<sg::Camera>> camera_components{
      ParseCameraComponents(tinygltf_model.cameras)};
  scene->SetComponents(std::move(camera_components));

  std::vector<std::unique_ptr<sg::Image>> image_components{
      ParseImageComponents(tinygltf_model.images, model.GetUriTextureMap())};
  scene->SetComponents(std::move(image_components));

  std::vector<std::unique_ptr<sg::Sampler>> sampler_components{
      ParseSamplerComponents(tinygltf_model.samplers)};
  scene->SetComponents(std::move(sampler_components));

  std::vector<std::unique_ptr<sg::Texture>> texture_components{
      ParseTextureComponents(tinygltf_model.textures, scene)};
  scene->SetComponents(std::move(texture_components));

  std::vector<std::unique_ptr<sg::Material>> material_components{
      ParseMaterialComponents(tinygltf_model.materials, scene)};
  scene->SetComponents(std::move(material_components));

  std::vector<std::unique_ptr<sg::Buffer>> buffer_components{
      ParseBufferComponents(tinygltf_model.buffers)};
  scene->SetComponents(std::move(buffer_components));

  std::vector<std::unique_ptr<sg::BufferView>> buffer_view_components{
      ParseBufferViewComponents(tinygltf_model.bufferViews, scene)};
  scene->SetComponents(std::move(buffer_view_components));

  std::vector<std::unique_ptr<sg::Accessor>> accesor_components{
      ParseAccessorComponents(tinygltf_model.accessors, scene)};
  scene->SetComponents(std::move(accesor_components));

  std::vector<std::unique_ptr<sg::Mesh>> mesh_components{
      ParseMeshComponents(tinygltf_model.meshes, scene)};
  scene->SetComponents(std::move(mesh_components));

  return scene;
}

std::unordered_map<std::string, bool> SceneGraph::ParseExtensionsUsed(
    const std::vector<std::string>& model_extensions_used) {
  std::unordered_map<std::string, bool> supported_extensions{
      {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

  for (const std::string& model_extension_used : model_extensions_used) {
    auto iter = supported_extensions.find(model_extension_used);
    if (iter == supported_extensions.end()) {
      THROW("Contain unsupported extension.");
    } else {
      iter->second = true;
    }
  }
  return supported_extensions;
}

std::vector<std::unique_ptr<sg::Light>> SceneGraph::ParseLightComponents(
    const tinygltf::ExtensionMap& model_extension_map,
    const std::unordered_map<std::string, bool>& supported_extensions) {
  std::vector<std::unique_ptr<sg::Light>> light_components;

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
            ParseLightComponent(model_lights.Get(static_cast<i32>(i)))};
        light_components.push_back(std::move(light_component));
      }
    }
  }
  return light_components;
}

std::vector<std::unique_ptr<sg::Camera>> SceneGraph::ParseCameraComponents(
    const std::vector<tinygltf::Camera>& model_cameras) {
  std::vector<std::unique_ptr<sg::Camera>> camera_components;

  for (const auto& model_camera : model_cameras) {
    auto camera_component{ParseCameraComponent(model_camera)};
    camera_components.push_back(std::move(camera_component));
  }

  return camera_components;
}

std::vector<std::unique_ptr<sg::Image>> SceneGraph::ParseImageComponents(
    const std::vector<tinygltf::Image>& tinygltf_images,
    const std::map<std::string, luka::ast::Image>& model_uri_image_map) {
  std::vector<std::unique_ptr<sg::Image>> image_components;

  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginTempCommandBuffer()};

  u64 model_image_count{tinygltf_images.size()};

  std::vector<gpu::Buffer> staging_buffers;

  for (u64 i{0}; i < model_image_count; ++i) {
    const tinygltf::Image& tinygltf_image{tinygltf_images[i]};

    const std::string& model_image_uri{tinygltf_image.uri};
    auto u2i{model_uri_image_map.find(model_image_uri)};
    if (u2i == model_uri_image_map.end()) {
      THROW("Fail to find image uri");
    }
    const ast::Image& model_image{u2i->second};
    auto image_component{
        ParseImageComponent(model_image, command_buffer, staging_buffers)};
    image_components.push_back(std::move(image_component));
  }

  ast::Image default_model_image{
      std::vector<u8>(4, 0),
      vk::Format::eR8G8B8A8Unorm,
      std::vector<ast::Mipmap>{{0, vk::Extent3D{1, 1, 1}}},
      1,
      1,
      {{{0}}},
      "default"};
  auto default_image_component{ParseImageComponent(
      default_model_image, command_buffer, staging_buffers)};
  image_components.push_back(std::move(default_image_component));

  gpu_->EndTempCommandBuffer(command_buffer);

  return image_components;
}

std::vector<std::unique_ptr<sg::Sampler>> SceneGraph::ParseSamplerComponents(
    const std::vector<tinygltf::Sampler>& model_samplers) {
  std::vector<std::unique_ptr<sg::Sampler>> sampler_components;
  for (const auto& model_sampler : model_samplers) {
    auto sampler_component{ParseSamplerComponent(model_sampler)};
    sampler_components.push_back(std::move(sampler_component));
  }

  tinygltf::Sampler default_model_sampler;
  default_model_sampler.name = "default";
  default_model_sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
  default_model_sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
  default_model_sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
  default_model_sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
  auto default_sampler_component{ParseSamplerComponent(default_model_sampler)};
  sampler_components.push_back(std::move(default_sampler_component));

  return sampler_components;
}

std::vector<std::unique_ptr<sg::Texture>> SceneGraph::ParseTextureComponents(
    const std::vector<tinygltf::Texture>& model_textures,
    const std::unique_ptr<sg::Scene>& scene) {
  std::vector<std::unique_ptr<sg::Texture>> texture_components;

  for (const auto& model_texture : model_textures) {
    auto texture_component{ParseTextureComponent(model_texture, scene)};
    texture_components.push_back(std::move(texture_component));
  }

  tinygltf::Texture default_model_texture;
  default_model_texture.name = "default";
  auto default_texture_component{
      ParseTextureComponent(default_model_texture, scene)};
  texture_components.push_back(std::move(default_texture_component));

  return texture_components;
}

std::vector<std::unique_ptr<sg::Material>> SceneGraph::ParseMaterialComponents(
    const std::vector<tinygltf::Material>& model_materials,
    const std::unique_ptr<sg::Scene>& scene) {
  std::vector<std::unique_ptr<sg::Material>> material_components;

  for (const auto& model_material : model_materials) {
    auto material_component{ParseMaterialComponent(model_material, scene)};
    material_components.push_back(std::move(material_component));
  }

  tinygltf::Material default_model_material;
  default_model_material.name = "default";
  auto default_material_component{
      ParseMaterialComponent(default_model_material, scene)};
  material_components.push_back(std::move(default_material_component));

  return material_components;
}

std::vector<std::unique_ptr<sg::Buffer>> SceneGraph::ParseBufferComponents(
    const std::vector<tinygltf::Buffer>& model_buffers) {
  std::vector<std::unique_ptr<sg::Buffer>> buffer_components;

  for (const auto& model_buffer : model_buffers) {
    std::unique_ptr<sg::Buffer> buffer_component{
        ParseBufferComponent(model_buffer)};

    buffer_components.push_back(std::move(buffer_component));
  }

  return buffer_components;
}

std::vector<std::unique_ptr<sg::BufferView>>
SceneGraph::ParseBufferViewComponents(
    const std::vector<tinygltf::BufferView>& model_buffer_views,
    const std::unique_ptr<sg::Scene>& scene) {
  std::vector<std::unique_ptr<sg::BufferView>> buffer_view_components;

  for (const auto& model_buffer_view : model_buffer_views) {
    std::unique_ptr<sg::BufferView> buffer_view_component{
        ParseBufferViewComponent(model_buffer_view, scene)};

    buffer_view_components.push_back(std::move(buffer_view_component));
  }

  return buffer_view_components;
}

std::vector<std::unique_ptr<sg::Accessor>> SceneGraph::ParseAccessorComponents(
    const std::vector<tinygltf::Accessor>& model_accessors,
    const std::unique_ptr<sg::Scene>& scene) {
  std::vector<std::unique_ptr<sg::Accessor>> accessor_compoents;

  for (const auto& model_accessor : model_accessors) {
    std::unique_ptr<sg::Accessor> accessor_compoent{
        ParseAccessorComponent(model_accessor, scene)};

    accessor_compoents.push_back(std::move(accessor_compoent));
  }

  return accessor_compoents;
}

std::vector<std::unique_ptr<sg::Mesh>> SceneGraph::ParseMeshComponents(
    const std::vector<tinygltf::Mesh>& model_meshs,
    const std::unique_ptr<sg::Scene>& scene) {
  std::vector<std::unique_ptr<sg::Mesh>> mesh_components;

  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginTempCommandBuffer()};

  std::vector<gpu::Buffer> staging_buffers;

  for (const auto& model_mesh : model_meshs) {
    std::unique_ptr<sg::Mesh> mesh_component{
        ParseMeshComponent(model_mesh, scene, command_buffer, staging_buffers)};
    mesh_components.push_back(std::move(mesh_component));
  }

  gpu_->EndTempCommandBuffer(command_buffer);

  return mesh_components;
}

std::unique_ptr<sg::Light> SceneGraph::ParseLightComponent(
    const tinygltf::Value& model_light) {
  sg::LightProperty property;
  if (!model_light.Has("type")) {
    THROW("Light doesn't have type.");
  }
  const std::string& type{model_light.Get("type").Get<std::string>()};
  if (type == "directional") {
    property.type = sg::LightType::kDirectional;
  } else if (type == "point") {
    property.type = sg::LightType::kPoint;
  } else if (type == "spot") {
    property.type = sg::LightType::kSpot;
  } else {
    THROW("Unkonwn light type.");
  }

  if (model_light.Has("color")) {
    property.color =
        glm::vec3(static_cast<f32>(model_light.Get("color").Get(0).Get<f64>()),
                  static_cast<f32>(model_light.Get("color").Get(1).Get<f64>()),
                  static_cast<f32>(model_light.Get("color").Get(2).Get<f64>()));
  }

  if (model_light.Has("intensity")) {
    property.intensity =
        static_cast<f32>(model_light.Get("intensity").Get<f64>());
  }

  if (property.type == sg::LightType::kPoint ||
      property.type == sg::LightType::kSpot) {
    property.range = static_cast<f32>(model_light.Get("range").Get<f64>());
  }

  if (property.type == sg::LightType::kSpot) {
    if (!model_light.Has("spot")) {
      THROW("Spot light doesn't have spot property.");
    }
    if (model_light.Get("spot").Has("innerConeAngle")) {
      property.inner_cone_angle = static_cast<f32>(
          model_light.Get("spot").Get("innerConeAngle").Get<f64>());
    }

    if (model_light.Get("spot").Has("outerConeAngle")) {
      property.outer_cone_angle = static_cast<f32>(
          model_light.Get("spot").Get("outerConeAngle").Get<f64>());
    }
  }

  std::string name;
  if (model_light.Has("name")) {
    name = model_light.Get("name").Get<std::string>();
  }
  auto light_component{std::make_unique<sg::Light>(property, name)};

  return light_component;
}

std::unique_ptr<sg::Camera> SceneGraph::ParseCameraComponent(
    const tinygltf::Camera& model_camera) {
  std::unique_ptr<sg::Camera> camera;

  const std::string& type{model_camera.type};
  const std::string& name{model_camera.name};

  if (type == "perspective") {
    const tinygltf::PerspectiveCamera& perspective{model_camera.perspective};
    f32 aspect_ratio{static_cast<f32>(perspective.aspectRatio)};
    f32 yfov{static_cast<f32>(perspective.yfov)};
    f32 znear{static_cast<f32>(perspective.znear)};
    f32 zfar{static_cast<f32>(perspective.zfar)};

    camera = std::make_unique<sg::PerspectiveCamera>(aspect_ratio, yfov, znear,
                                                     zfar, name);

  } else {
    THROW("Unsupport camera type");
  }

  return camera;
}

std::unique_ptr<sg::Image> SceneGraph::ParseImageComponent(
    const ast::Image& model_image,
    const vk::raii::CommandBuffer& command_buffer,
    std::vector<gpu::Buffer>& staging_buffers) {
  // Staging buffer.
  const auto& data{model_image.GetDate()};
  u64 data_size{data.size()};

  vk::BufferCreateInfo image_staging_buffer_ci{
      {}, data_size, vk::BufferUsageFlagBits::eTransferSrc};

  gpu::Buffer image_staging_buffer{
      gpu_->CreateBuffer(image_staging_buffer_ci, data.data())};
  staging_buffers.push_back(std::move(image_staging_buffer));

  // Image.
  const auto& mipmaps{model_image.GetMipmaps()};
  if (mipmaps.empty()) {
    THROW("Mipmaps is empty");
  }
  vk::Extent3D extent{mipmaps[0].extent};
  u32 dim_count{0};
  vk::ImageType image_type;
  if (extent.width >= 1) {
    ++dim_count;
  }
  if (extent.height >= 1) {
    ++dim_count;
  }
  if (extent.depth > 1) {
    ++dim_count;
  }
  switch (dim_count) {
    case 1:
      image_type = vk::ImageType::e1D;
      break;
    case 2:
      image_type = vk::ImageType::e2D;
      break;
    case 3:
      image_type = vk::ImageType::e3D;
      break;
    default:
      image_type = vk::ImageType::e2D;
      break;
  }

  vk::Format format{model_image.GetFormat()};
  u32 mipmap_count{static_cast<u32>(mipmaps.size())};
  u32 layer_count{model_image.GetLayerCount()};

  vk::ImageCreateInfo image_ci{
      {},
      image_type,
      format,
      extent,
      mipmap_count,
      layer_count,
      vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst};

  const std::string& name{model_image.GetName()};

  gpu::Image image{gpu_->CreateImage(
      image_ci, vk::ImageLayout::eShaderReadOnlyOptimal, staging_buffers.back(),
      command_buffer, model_image, name)};

  // Image view.
  vk::ImageViewType image_view_type;
  switch (image_type) {
    case vk::ImageType::e1D:
      image_view_type = vk::ImageViewType::e1D;
      break;
    case vk::ImageType::e2D:
      image_view_type = vk::ImageViewType::e2D;
      break;
    case vk::ImageType::e3D:
      image_view_type = vk::ImageViewType::e3D;
      break;
    default:
      image_view_type = vk::ImageViewType::e2D;
      break;
  }

  vk::ImageViewCreateInfo image_view_ci{
      {},
      *image,
      image_view_type,
      format,
      {},
      {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
       VK_REMAINING_ARRAY_LAYERS}};

  vk::raii::ImageView image_view{gpu_->CreateImageView(image_view_ci, name)};

  auto image_component{std::make_unique<sg::Image>(
      std::move(image), std::move(image_view), name)};

  return image_component;
}

std::unique_ptr<sg::Sampler> SceneGraph::ParseSamplerComponent(
    const tinygltf::Sampler& model_sampler) {
  vk::Filter mag_filter;
  switch (model_sampler.minFilter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
      mag_filter = vk::Filter::eNearest;
      break;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
      mag_filter = vk::Filter::eLinear;
      break;
    default:
      mag_filter = vk::Filter::eNearest;
  }

  vk::Filter min_filter;
  switch (model_sampler.minFilter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
      min_filter = vk::Filter::eNearest;
      break;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      min_filter = vk::Filter::eLinear;
      break;
    default:
      min_filter = vk::Filter::eNearest;
  }

  vk::SamplerMipmapMode mipmap_mode;
  switch (model_sampler.minFilter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
      mipmap_mode = vk::SamplerMipmapMode::eNearest;
      break;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      mipmap_mode = vk::SamplerMipmapMode::eLinear;
      break;
    default:
      mipmap_mode = vk::SamplerMipmapMode::eNearest;
  }

  vk::SamplerAddressMode address_mode_u;
  switch (model_sampler.wrapS) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      address_mode_u = vk::SamplerAddressMode::eRepeat;
      break;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      address_mode_u = vk::SamplerAddressMode::eClampToEdge;
      break;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      address_mode_u = vk::SamplerAddressMode::eMirroredRepeat;
      break;
    default:
      address_mode_u = vk::SamplerAddressMode::eRepeat;
  }

  vk::SamplerAddressMode address_mode_v;
  switch (model_sampler.wrapT) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      address_mode_v = vk::SamplerAddressMode::eRepeat;
      break;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      address_mode_v = vk::SamplerAddressMode::eClampToEdge;
      break;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      address_mode_v = vk::SamplerAddressMode::eMirroredRepeat;
      break;
    default:
      address_mode_v = vk::SamplerAddressMode::eRepeat;
  }

  vk::SamplerCreateInfo sampler_ci{{},          mag_filter,     min_filter,
                                   mipmap_mode, address_mode_u, address_mode_v};

  const std::string& name{model_sampler.name};

  vk::raii::Sampler sampler{gpu_->CreateSampler(sampler_ci, name)};

  auto sampler_component{
      std::make_unique<sg::Sampler>(std::move(sampler), name)};

  return sampler_component;
}

std::unique_ptr<sg::Texture> SceneGraph::ParseTextureComponent(
    const tinygltf::Texture& model_texture,
    const std::unique_ptr<sg::Scene>& scene) {
  auto image_components{scene->GetComponents<sg::Image>()};
  auto sampler_components{scene->GetComponents<sg::Sampler>()};

  sg::Image* image;
  if (model_texture.source != -1) {
    image = image_components[model_texture.source];
  } else {
    image = image_components.back();
  }

  sg::Sampler* sampler;
  if (model_texture.sampler != -1) {
    sampler = sampler_components[model_texture.sampler];
  } else {
    sampler = sampler_components.back();
  }

  const std::string& name{model_texture.name};

  auto texture_component{std::make_unique<sg::Texture>(image, sampler, name)};

  return texture_component;
}

std::unique_ptr<sg::Material> SceneGraph::ParseMaterialComponent(
    const tinygltf::Material& model_material,
    const std::unique_ptr<sg::Scene>& scene) {
  auto texture_components{scene->GetComponents<sg::Texture>()};

  // Pbr.
  const tinygltf::PbrMetallicRoughness& metallic_roughness{
      model_material.pbrMetallicRoughness};

  std::vector<f32> base_color_factor_fv{
      D2FVector(metallic_roughness.baseColorFactor)};
  glm::vec4 base_color_factor{glm::make_vec4(base_color_factor_fv.data())};

  sg::Texture* base_color_texture;
  if (metallic_roughness.baseColorTexture.index != -1) {
    base_color_texture =
        texture_components[metallic_roughness.baseColorTexture.index];
  } else {
    base_color_texture = texture_components.back();
  }

  f32 metallic_factor{static_cast<f32>(metallic_roughness.metallicFactor)};

  f32 roughness_factor{static_cast<f32>(metallic_roughness.roughnessFactor)};

  sg::Texture* metallic_roughness_texture;
  if (metallic_roughness.metallicRoughnessTexture.index != -1) {
    metallic_roughness_texture =
        texture_components[metallic_roughness.metallicRoughnessTexture.index];
  } else {
    metallic_roughness_texture = texture_components.back();
  }

  // Normal.
  const tinygltf::NormalTextureInfo& normal{model_material.normalTexture};

  sg::Texture* normal_texture;
  if (normal.index != -1) {
    normal_texture = texture_components[normal.index];
  } else {
    normal_texture = texture_components.back();
  }

  f32 scale{static_cast<f32>(normal.scale)};

  // Occlusion.
  const tinygltf::OcclusionTextureInfo& occlusion{
      model_material.occlusionTexture};

  sg::Texture* occlusion_texture;
  if (occlusion.index != -1) {
    occlusion_texture = texture_components[occlusion.index];
  } else {
    occlusion_texture = texture_components.back();
  }

  f32 strength{static_cast<f32>(occlusion.strength)};

  // Emissive.
  std::vector<f32> emissive_factor_fv{D2FVector(model_material.emissiveFactor)};
  glm::vec3 emissive_factor{glm::make_vec3(emissive_factor_fv.data())};

  sg::Texture* emissive_texture;
  if (model_material.emissiveTexture.index != -1) {
    emissive_texture = texture_components[model_material.emissiveTexture.index];
  } else {
    emissive_texture = texture_components.back();
  }

  // Others.
  const std::string& alpha_mode_str{model_material.alphaMode};
  sg::AlphaMode alpha_mode;
  if (alpha_mode_str == "OPAQUE") {
    alpha_mode = sg::AlphaMode::kOpaque;
  } else if (alpha_mode_str == "MASK") {
    alpha_mode = sg::AlphaMode::kMask;
  } else if (alpha_mode_str == "BLEND") {
    alpha_mode = sg::AlphaMode::kBlend;
  } else {
    THROW("Unsupported alpha mode");
  }

  f32 alpha_cutoff{static_cast<f32>(model_material.alphaCutoff)};

  bool double_sided{model_material.doubleSided};

  const std::string& name{model_material.name};

  // Construct material.
  std::array<sg::Texture*, 5> material_textures{
      base_color_texture, metallic_roughness_texture, normal_texture,
      occlusion_texture, emissive_texture};

  auto material_component{std::make_unique<sg::Material>(
      std::move(material_textures), std::move(base_color_factor),
      metallic_factor, roughness_factor, scale, strength,
      std::move(emissive_factor), alpha_mode, alpha_cutoff, double_sided,
      name)};

  return material_component;
}

std::unique_ptr<sg::Buffer> SceneGraph::ParseBufferComponent(
    const tinygltf::Buffer& model_buffer) {
  const std::string& name{model_buffer.name};
  const auto* data{&(model_buffer.data)};

  auto buffer_component{std::make_unique<sg::Buffer>(data, name)};
  return buffer_component;
}

std::unique_ptr<sg::BufferView> SceneGraph::ParseBufferViewComponent(
    const tinygltf::BufferView& model_buffer_view,
    const std::unique_ptr<sg::Scene>& scene) {
  auto buffer_componens{scene->GetComponents<sg::Buffer>()};

  sg::Buffer* buffer{buffer_componens[model_buffer_view.buffer]};

  u64 byte_offset{model_buffer_view.byteOffset};

  u64 byte_length{model_buffer_view.byteLength};

  u64 byte_stride{model_buffer_view.byteStride};

  const std::string& name{model_buffer_view.name};

  auto buffer_view_component{std::make_unique<sg::BufferView>(
      buffer, byte_offset, byte_length, byte_stride, name)};

  return buffer_view_component;
}

std::unique_ptr<sg::Accessor> SceneGraph::ParseAccessorComponent(
    const tinygltf::Accessor& model_accessor,
    const std::unique_ptr<sg::Scene>& scene) {
  auto buffer_view_components{scene->GetComponents<sg::BufferView>()};

  sg::BufferView* buffer_view{
      buffer_view_components[model_accessor.bufferView]};

  u64 byte_offset{model_accessor.byteOffset};

  bool normalized{model_accessor.normalized};

  u32 component_type{static_cast<u32>(model_accessor.componentType)};

  u64 count{model_accessor.count};

  u32 type{static_cast<u32>(model_accessor.type)};

  const std::string& name{model_accessor.name};

  auto accessor_component{std::make_unique<sg::Accessor>(
      buffer_view, byte_offset, normalized, component_type, count, type, name)};

  return accessor_component;
}

std::unique_ptr<sg::Mesh> SceneGraph::ParseMeshComponent(
    const tinygltf::Mesh& model_mesh, const std::unique_ptr<sg::Scene>& scene,
    const vk::raii::CommandBuffer& command_buffer,
    std::vector<gpu::Buffer>& staging_buffers) {
  const std::string& name{model_mesh.name};
  const std::vector<tinygltf::Primitive> model_primitives{
      model_mesh.primitives};

  auto accessor_components{scene->GetComponents<sg::Accessor>()};

  std::vector<sg::Primitive> primitives;

  std::vector<gpu::Buffer> buffers;

  for (const auto& model_primitive : model_primitives) {
    sg::Primitive primitive;

    for (const auto& attribute : model_primitive.attributes) {
      const std::string& attribute_name{attribute.first};
      u32 attribute_accessor_index{static_cast<u32>(attribute.second)};
      sg::Accessor* accessor{accessor_components[attribute_accessor_index]};

      auto accessor_buffer{accessor->GetBuffer()};
      const u8* buffer_data{accessor_buffer.first};
      u64 buffer_size{accessor_buffer.second};

      vk::BufferCreateInfo staging_buffer_ci{
          {}, buffer_size, vk::BufferUsageFlagBits::eTransferSrc};

      vk::BufferCreateInfo buffer_ci{{},
                                     buffer_size,
                                     vk::BufferUsageFlagBits::eVertexBuffer |
                                         vk::BufferUsageFlagBits::eIndexBuffer |
                                         vk::BufferUsageFlagBits::eTransferDst,
                                     vk::SharingMode::eExclusive};

      gpu::Buffer staging_buffer{
          gpu_->CreateBuffer(staging_buffer_ci, buffer_data)};
      staging_buffers.push_back(std::move(staging_buffer));

      gpu::Buffer buffer{gpu_->CreateBuffer(buffer_ci, staging_buffers.back(),
                                            command_buffer)};
      primitive.vertex_buffers.insert(
          std::make_pair(attribute_name, std::move(buffer)));
    }

    primitives.emplace_back(std::move(primitive));
  }

  auto mesh_component{std::make_unique<sg::Mesh>(std::move(primitives))};
  return mesh_component;
}

}  // namespace luka
