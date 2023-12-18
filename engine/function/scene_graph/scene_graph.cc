// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene_graph.h"

#include "context.h"
#include "core/log.h"
#include "function/scene_graph/component/light.h"

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

  // Extension.
  std::unordered_map<std::string, bool> supported_extensions{
      {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

  for (const auto& extension_used : model.extensionsUsed) {
    auto iter = supported_extensions.find(extension_used);
    if (iter == supported_extensions.end()) {
      THROW("Contain unsupported extension.");
    } else {
      iter->second = true;
    }
  }

  // Light.
  std::vector<std::unique_ptr<sg::Light>> lights;
  if (supported_extensions[KHR_LIGHTS_PUNCTUAL_EXTENSION]) {
    if (model.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) !=
            model.extensions.end() &&
        model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Has("lights")) {
      const auto& khr_lights{
          model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Get("lights")};

      for (u64 i{0}; i < khr_lights.ArrayLen(); ++i) {
        const auto& khr_light{khr_lights.Get(static_cast<int>(i))};

        sg::LightProperty property;
        if (!khr_light.Has("type")) {
          THROW("Light doesn't have type.");
        }
        const auto& khr_type{khr_light.Get("type").Get<std::string>()};
        if (khr_type == "directional") {
          property.type = sg::LightType::kDirectional;
        } else if (khr_type == "point") {
          property.type = sg::LightType::kPoint;
        } else if (khr_type == "spot") {
          property.type = sg::LightType::kSpot;
        } else {
          THROW("Unkonwn light type.");
        }

        if (khr_light.Has("color")) {
          property.color = glm::vec3(
              static_cast<float>(khr_light.Get("color").Get(0).Get<double>()),
              static_cast<float>(khr_light.Get("color").Get(1).Get<double>()),
              static_cast<float>(khr_light.Get("color").Get(2).Get<double>()));
        }

        if (khr_light.Has("intensity")) {
          property.intensity =
              static_cast<float>(khr_light.Get("intensity").Get<double>());
        }

        if (property.type == sg::LightType::kPoint ||
            property.type == sg::LightType::kSpot) {
          property.range =
              static_cast<float>(khr_light.Get("range").Get<double>());
        }

        if (property.type == sg::LightType::kSpot) {
          if (!khr_light.Has("spot")) {
            THROW("Spot light doesn't have spot property.");
          }
          if (khr_light.Get("spot").Has("innerConeAngle")) {
            property.inner_cone_angle = static_cast<float>(
                khr_light.Get("spot").Get("innerConeAngle").Get<double>());
          }

          if (khr_light.Get("spot").Has("outerConeAngle")) {
            property.outer_cone_angle = static_cast<float>(
                khr_light.Get("spot").Get("outerConeAngle").Get<double>());
          }
        }

        std::string name;
        if (khr_light.Has("name")) {
          name = khr_light.Get("name").Get<std::string>();
        }
        auto light{std::make_unique<sg::Light>(property, name)};

        lights.push_back(std::move(light));
      }
    }
  }
  scene->SetComponents(std::move(lights));

  // Sampler.

  return scene;
}

}  // namespace luka
