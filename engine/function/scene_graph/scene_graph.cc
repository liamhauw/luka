// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene_graph.h"

#include "context.h"
#include "core/log.h"
#include "function/scene_graph/component/light.h"
#include "function/scene_graph/component/sampler.h"

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace luka {

SceneGraph::SceneGraph() {
  const AssetInfo& asset_info{gContext.asset->GetAssetInfo()};
  const tinygltf::Model& skybox{asset_info.skybox};
  const tinygltf::Model& object{asset_info.object};

  // skybox_ = LoadScene(skybox);
  object_ = LoadScene(object);
}

void SceneGraph::Tick() {}

std::unique_ptr<sg::Scene> SceneGraph::LoadScene(const tinygltf::Model& model) {
  auto scene{std::make_unique<sg::Scene>("gltf_scene")};

  auto gpu{gContext.gpu};

  // Extensions used.
  std::unordered_map<std::string, bool> supported_extensions{
      {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

  for (const std::string& extension_used : model.extensionsUsed) {
    auto iter = supported_extensions.find(extension_used);
    if (iter == supported_extensions.end()) {
      THROW("Contain unsupported extension.");
    } else {
      iter->second = true;
    }
  }

  // Lights.
  std::vector<std::unique_ptr<sg::Light>> light_components;
  if (supported_extensions[KHR_LIGHTS_PUNCTUAL_EXTENSION]) {
    if (model.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) !=
            model.extensions.end() &&
        model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Has("lights")) {
      const auto& lights{
          model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Get("lights")};

      for (u64 i{0}; i < lights.ArrayLen(); ++i) {
        const auto& light{lights.Get(static_cast<int>(i))};

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
          property.color = glm::vec3(
              static_cast<float>(light.Get("color").Get(0).Get<double>()),
              static_cast<float>(light.Get("color").Get(1).Get<double>()),
              static_cast<float>(light.Get("color").Get(2).Get<double>()));
        }

        if (light.Has("intensity")) {
          property.intensity =
              static_cast<float>(light.Get("intensity").Get<double>());
        }

        if (property.type == sg::LightType::kPoint ||
            property.type == sg::LightType::kSpot) {
          property.range = static_cast<float>(light.Get("range").Get<double>());
        }

        if (property.type == sg::LightType::kSpot) {
          if (!light.Has("spot")) {
            THROW("Spot light doesn't have spot property.");
          }
          if (light.Get("spot").Has("innerConeAngle")) {
            property.inner_cone_angle = static_cast<float>(
                light.Get("spot").Get("innerConeAngle").Get<double>());
          }

          if (light.Get("spot").Has("outerConeAngle")) {
            property.outer_cone_angle = static_cast<float>(
                light.Get("spot").Get("outerConeAngle").Get<double>());
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
  scene->SetComponents(std::move(light_components));

  // Samplers.
  std::vector<std::unique_ptr<sg::Sampler>> sampler_components;
  for (const auto& sampler : model.samplers) {
    vk::Filter mag_filter{ParseMagFilter(sampler.magFilter)};
    vk::Filter min_filter{ParseMinFilter(sampler.minFilter)};
    vk::SamplerMipmapMode mipmap_mode{ParseMipmapMode(sampler.minFilter)};
    vk::SamplerAddressMode address_mode_u{ParseAddressMode(sampler.wrapS)};
    vk::SamplerAddressMode address_mode_v{ParseAddressMode(sampler.wrapT)};

    vk::SamplerCreateInfo sampler_ci{
        {},          mag_filter,     min_filter,
        mipmap_mode, address_mode_u, address_mode_v};

    const std::string& name{sampler.name};

    vk::raii::Sampler model_sampler{gpu->CreateSampler(sampler_ci, name)};

    auto sampler_component{std::make_unique<sg::Sampler>(std::move(model_sampler), name)};

    sampler_components.push_back(std::move(sampler_component));
  }
  scene->SetComponents(std::move(sampler_components));

  return scene;
}

vk::Filter SceneGraph::ParseMagFilter(int mag_filter) {
  switch (mag_filter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
      return vk::Filter::eNearest;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
      return vk::Filter::eLinear;
    default:
      return vk::Filter::eNearest;
  }
}

vk::Filter SceneGraph::ParseMinFilter(int min_filter) {
  switch (min_filter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
      return vk::Filter::eNearest;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      return vk::Filter::eLinear;
    default:
      return vk::Filter::eNearest;
  }
}

vk::SamplerMipmapMode SceneGraph::ParseMipmapMode(int min_filter) {
  switch (min_filter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
      return vk::SamplerMipmapMode::eNearest;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      return vk::SamplerMipmapMode::eLinear;
    default:
      return vk::SamplerMipmapMode::eNearest;
  }
}

vk::SamplerAddressMode SceneGraph::ParseAddressMode(int wrap) {
  switch (wrap) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      return vk::SamplerAddressMode::eRepeat;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      return vk::SamplerAddressMode::eClampToEdge;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      return vk::SamplerAddressMode::eMirroredRepeat;
    default:
      return vk::SamplerAddressMode::eRepeat;
  }
}

}  // namespace luka
