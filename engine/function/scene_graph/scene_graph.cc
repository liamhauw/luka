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

  // skybox_ = LoadScene(skybox);
  object_ = LoadScene(object);
}

void SceneGraph::Tick() {}

std::unique_ptr<sg::Scene> SceneGraph::LoadScene(const ast::Model& model) {
  auto scene{std::make_unique<sg::Scene>("gltf_scene")};

  std::unordered_map<std::string, bool> supported_extensions{
      ParseExtensionsUsed(model)};

  std::vector<std::unique_ptr<sg::Light>> light_components{
      ParseLights(model, supported_extensions)};
  scene->SetComponents(std::move(light_components));

  std::vector<std::unique_ptr<sg::Sampler>> sampler_components{
      ParseSamplers(model)};
  scene->SetComponents(std::move(sampler_components));

  std::vector<std::unique_ptr<sg::Image>> image_components{ParseImages(model)};
  scene->SetComponents(std::move(image_components));

  return scene;
}

std::unordered_map<std::string, bool> SceneGraph::ParseExtensionsUsed(
    const ast::Model& model) {
  std::unordered_map<std::string, bool> supported_extensions{
      {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

  for (const std::string& extension_used :
       model.GetTinygltfModel().extensionsUsed) {
    auto iter = supported_extensions.find(extension_used);
    if (iter == supported_extensions.end()) {
      THROW("Contain unsupported extension.");
    } else {
      iter->second = true;
    }
  }
  return supported_extensions;
}

std::vector<std::unique_ptr<sg::Light>> SceneGraph::ParseLights(
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
      const auto& lights{model.GetTinygltfModel()
                             .extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION)
                             .Get("lights")};

      for (u64 i{0}; i < lights.ArrayLen(); ++i) {
        const auto& light{lights.Get(static_cast<i32>(i))};

        sg::LightProperty property;
        if (!light.Has("type")) {
          THROW("Light doesn't have type.");
        }
        const std::string& type{light.Get("type").Get<std::string>()};
        if (type == "directional") {
          property.type = sg::LightType::kDirectional;
        } else if (type == "point") {
          property.type = sg::LightType::kPoint;
        } else if (type == "spot") {
          property.type = sg::LightType::kSpot;
        } else {
          THROW("Unkonwn light type.");
        }

        if (light.Has("color")) {
          property.color =
              glm::vec3(static_cast<f32>(light.Get("color").Get(0).Get<f64>()),
                        static_cast<f32>(light.Get("color").Get(1).Get<f64>()),
                        static_cast<f32>(light.Get("color").Get(2).Get<f64>()));
        }

        if (light.Has("intensity")) {
          property.intensity =
              static_cast<f32>(light.Get("intensity").Get<f64>());
        }

        if (property.type == sg::LightType::kPoint ||
            property.type == sg::LightType::kSpot) {
          property.range = static_cast<f32>(light.Get("range").Get<f64>());
        }

        if (property.type == sg::LightType::kSpot) {
          if (!light.Has("spot")) {
            THROW("Spot light doesn't have spot property.");
          }
          if (light.Get("spot").Has("innerConeAngle")) {
            property.inner_cone_angle = static_cast<f32>(
                light.Get("spot").Get("innerConeAngle").Get<f64>());
          }

          if (light.Get("spot").Has("outerConeAngle")) {
            property.outer_cone_angle = static_cast<f32>(
                light.Get("spot").Get("outerConeAngle").Get<f64>());
          }
        }

        std::string name;
        if (light.Has("name")) {
          name = light.Get("name").Get<std::string>();
        }
        auto light_component{std::make_unique<sg::Light>(property, name)};

        light_components.push_back(std::move(light_component));
      }
    }
  }
  return light_components;
}

std::vector<std::unique_ptr<sg::Sampler>> SceneGraph::ParseSamplers(
    const ast::Model& model) {
  std::vector<std::unique_ptr<sg::Sampler>> sampler_components;
  for (const auto& sampler : model.GetTinygltfModel().samplers) {
    vk::Filter mag_filter;
    switch (sampler.minFilter) {
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
    switch (sampler.minFilter) {
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
    switch (sampler.minFilter) {
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
    switch (sampler.wrapS) {
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
    switch (sampler.wrapT) {
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

    const std::string& name{sampler.name};

    vk::raii::Sampler model_sampler{gpu_->CreateSampler(sampler_ci, name)};

    auto sampler_component{
        std::make_unique<sg::Sampler>(std::move(model_sampler), name)};

    sampler_components.push_back(std::move(sampler_component));
  }

  return sampler_components;
}

std::vector<std::unique_ptr<sg::Image>> SceneGraph::ParseImages(
    const ast::Model& model) {
  std::vector<std::unique_ptr<sg::Image>> image_components;

  u32 thread_count{std::thread::hardware_concurrency()};
  thread_count = thread_count == 0 ? 1 : thread_count;
  ctpl::thread_pool thread_pool{static_cast<i32>(thread_count)};
  std::vector<std::future<std::unique_ptr<sg::Image>>> future_image_components;

  u64 image_count{model.GetTinygltfModel().images.size()};
  for (u64 i{0}; i < image_count; ++i) {
    auto future_image{thread_pool.push([this, i](u64) {})};
  }

  return image_components;
}

}  // namespace luka
