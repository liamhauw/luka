// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene_graph.h"

#include <ctpl_stl.h>

#include "context.h"
#include "core/log.h"

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace luka {

SceneGraph::SceneGraph() : gpu_{gContext.gpu} {
  const AssetInfo& asset_info{gContext.asset->GetAssetInfo()};
  const ast::Model& skybox{asset_info.skybox};
  const ast::Model& object{asset_info.object};

  skybox_ = LoadScene(skybox);
  object_ = LoadScene(object);
}

void SceneGraph::Tick() {}

std::unique_ptr<sg::Scene> SceneGraph::LoadScene(const ast::Model& model) {
  auto scene{std::make_unique<sg::Scene>("gltf_scene")};

  std::unordered_map<std::string, bool> supported_extensions{
      ParseExtensionsUsed(model)};

  std::vector<std::unique_ptr<sg::Light>> light_components{
      ParseLightComponents(model, supported_extensions)};
  scene->SetComponents(std::move(light_components));

  std::vector<std::unique_ptr<sg::Image>> image_components{
      ParseImageComponents(model)};
  scene->SetComponents(std::move(image_components));

  std::vector<std::unique_ptr<sg::Sampler>> sampler_components{
      ParseSamplerComponents(model)};
  scene->SetComponents(std::move(sampler_components));

  return scene;
}

std::unordered_map<std::string, bool> SceneGraph::ParseExtensionsUsed(
    const ast::Model& model) {
  std::unordered_map<std::string, bool> supported_extensions{
      {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

  for (const std::string& model_extension_used :
       model.GetTinygltfModel().extensionsUsed) {
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
    const ast::Model& model,
    const std::unordered_map<std::string, bool>& supported_extensions) {
  std::vector<std::unique_ptr<sg::Light>> light_components;

  auto khr_lights_punctual_extension{
      supported_extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION)};
  if (khr_lights_punctual_extension != supported_extensions.end() &&
      khr_lights_punctual_extension->second) {
    if (model.GetTinygltfModel().extensions.find(
            KHR_LIGHTS_PUNCTUAL_EXTENSION) !=
            model.GetTinygltfModel().extensions.end() &&
        model.GetTinygltfModel()
            .extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION)
            .Has("lights")) {
      const auto& model_lights{model.GetTinygltfModel()
                                   .extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION)
                                   .Get("lights")};

      for (u64 i{0}; i < model_lights.ArrayLen(); ++i) {
        const auto& model_light{model_lights.Get(static_cast<i32>(i))};

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
          property.color = glm::vec3(
              static_cast<f32>(model_light.Get("color").Get(0).Get<f64>()),
              static_cast<f32>(model_light.Get("color").Get(1).Get<f64>()),
              static_cast<f32>(model_light.Get("color").Get(2).Get<f64>()));
        }

        if (model_light.Has("intensity")) {
          property.intensity =
              static_cast<f32>(model_light.Get("intensity").Get<f64>());
        }

        if (property.type == sg::LightType::kPoint ||
            property.type == sg::LightType::kSpot) {
          property.range =
              static_cast<f32>(model_light.Get("range").Get<f64>());
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

        light_components.push_back(std::move(light_component));
      }
    }
  }
  return light_components;
}

// TODO Cube
std::vector<std::unique_ptr<sg::Image>> SceneGraph::ParseImageComponents(
    const ast::Model& model) {
  std::vector<std::unique_ptr<sg::Image>> image_components;

  const auto& model_images{model.GetTinygltfModel().images};
  const auto& model_uri_image_map{model.GetUriTextureMap()};

  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginTempCommandBuffer()};

  u64 model_image_count{model_images.size()};

  std::vector<gpu::Buffer> staging_buffers;

  for (u64 i{0}; i < model_image_count; ++i) {
    const std::string& model_image_uri{model_images[i].uri};
    auto u2i{model_uri_image_map.find(model_image_uri)};
    if (u2i == model_uri_image_map.end()) {
      THROW("Fail to find image uri");
    }
    const ast::Image& model_image{u2i->second};

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

    vk::ImageCreateInfo image_ci{{},
                                 image_type,
                                 format,
                                 extent,
                                 mipmap_count,
                                 layer_count,
                                 vk::SampleCountFlagBits::e1,
                                 vk::ImageTiling::eOptimal,
                                 vk::ImageUsageFlagBits::eSampled |
                                     vk::ImageUsageFlagBits::eTransferDst};

    const auto& offsets{model_image.GetOffsets()};
    const std::string& name{model_images[i].name};

    gpu::Image image{gpu_->CreateImage(
        image_ci, vk::ImageLayout::eShaderReadOnlyOptimal, staging_buffers[i],
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

    auto image_component{
        std::make_unique<sg::Image>(std::move(image), std::move(image_view))};

    image_components.emplace_back(std::move(image_component));
  }

  gpu_->EndTempCommandBuffer(command_buffer);

  return image_components;
}

std::vector<std::unique_ptr<sg::Sampler>> SceneGraph::ParseSamplerComponents(
    const ast::Model& model) {
  std::vector<std::unique_ptr<sg::Sampler>> sampler_components;
  for (const auto& model_sampler : model.GetTinygltfModel().samplers) {
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

    vk::SamplerCreateInfo sampler_ci{
        {},          mag_filter,     min_filter,
        mipmap_mode, address_mode_u, address_mode_v};

    const std::string& name{model_sampler.name};

    vk::raii::Sampler sampler{gpu_->CreateSampler(sampler_ci, name)};

    auto sampler_component{
        std::make_unique<sg::Sampler>(std::move(sampler), name)};

    sampler_components.push_back(std::move(sampler_component));
  }

  return sampler_components;
}

std::vector<std::unique_ptr<sg::Texture>> SceneGraph::ParseTextureComponents(
    const ast::Model& model, const std::unique_ptr<sg::Scene>& scene) {
  auto image_components{scene->GetComponents<sg::Image>()};
  auto sampler_components{scene->GetComponents<sg::Sampler>()};

  for (const auto& model_texture : model.GetTinygltfModel().textures) {
    sg::Image* image{image_components[model_texture.source]};
    sg::Sampler* sampler{sampler_components[model_texture.sampler]};
  }

  return {};
}

}  // namespace luka
