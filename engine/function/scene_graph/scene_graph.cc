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
  auto default_material_component{ParseMaterialComponent(default_model_material, scene)};
  material_components.push_back(std::move(default_material_component));

  return material_components;
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

}  // namespace luka
