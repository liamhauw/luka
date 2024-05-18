// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/light.h"

#include "core/log.h"

namespace luka::ast {

Light::Light(const std::filesystem::path& light_path) {
  std::ifstream light_file{light_path.string()};
  if (!light_file) {
    THROW("Fail to load config file {}", light_path.string());
  }
  json_ = json::parse(light_file);

  if (json_.contains("punctual_lights")) {
    const json& punctual_lights_json{json_["punctual_lights"]};
    for (const auto& punctual_light_json : punctual_lights_json) {
      PunctualLight punctual_light{};

      if (punctual_light_json.contains("type")) {
        std::string type{
            punctual_light_json["type"].template get<std::string>()};

        if (type == "directional") {
          punctual_light.type =
              static_cast<u32>(PunctualLightType::kDirectional);
        } else if (type == "point") {
          punctual_light.type = static_cast<u32>(PunctualLightType::kPoint);
        } else if (type == "spot") {
          punctual_light.type = static_cast<u32>(PunctualLightType::kSpot);
        } else {
          LOGW("Unsupport light postion");
          continue;
        }
      }

      if (punctual_light_json.contains("position")) {
        const json& position{punctual_light_json["position"]};
        if (position.size() < 3) {
          LOGW("Unsupport light postion");
          continue;
        }
        punctual_light.position =
            glm::vec3{position[0], position[1], position[2]};
      }

      if (punctual_light_json.contains("intensity")) {
        punctual_light.intensity =
            punctual_light_json["intensity"].template get<f32>();
      }

      if (punctual_light_json.contains("color")) {
        const json& color{punctual_light_json["color"]};
        if (color.size() < 3) {
          LOGW("Unsupport light color");
          continue;
        }
        punctual_light.color = glm::vec3{color[0], color[1], color[2]};
      }

      if (punctual_light_json.contains("range")) {
        punctual_light.range = punctual_light_json["range"].template get<f32>();
      }

      puntual_lights_.push_back(punctual_light);
    }
  }
}

const std::vector<PunctualLight>& Light::GetPunctualLights() const {
  return puntual_lights_;
}

}  // namespace luka::ast
