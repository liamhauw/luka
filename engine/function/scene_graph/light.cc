// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/light.h"

#include "core/log.h"

namespace luka {
namespace sg {

Light::Light(LightType type, glm::vec3 direction, glm::vec3 color,
             f32 intensity, f32 range, f32 inner_cone_angle,
             f32 outer_cone_angle, const std::string& name)
    : Component{name},
      type_{type},
      direction_{direction},
      color_{color},
      intensity_{intensity},
      range_{range},
      inner_cone_angle_{inner_cone_angle},
      outer_cone_angle_{outer_cone_angle} {}

Light::Light(const tinygltf::Value& model_light) {
  if (model_light.Has("name")) {
    name_ = model_light.Get("name").Get<std::string>();
  }

  if (!model_light.Has("type")) {
    THROW("Light doesn't have type.");
  }
  const std::string& type{model_light.Get("type").Get<std::string>()};
  if (type == "directional") {
    type_ = LightType::kDirectional;
  } else if (type == "point") {
    type_ = LightType::kPoint;
  } else if (type == "spot") {
    type_ = LightType::kSpot;
  } else {
    THROW("Unkonwn light type.");
  }

  if (model_light.Has("color")) {
    color_ =
        glm::vec3(static_cast<f32>(model_light.Get("color").Get(0).Get<f64>()),
                  static_cast<f32>(model_light.Get("color").Get(1).Get<f64>()),
                  static_cast<f32>(model_light.Get("color").Get(2).Get<f64>()));
  }

  if (model_light.Has("intensity")) {
    intensity_ = static_cast<f32>(model_light.Get("intensity").Get<f64>());
  }

  if (type_ == LightType::kPoint || type_ == LightType::kSpot) {
    range_ = static_cast<f32>(model_light.Get("range").Get<f64>());
  }

  if (type_ == LightType::kSpot) {
    if (!model_light.Has("spot")) {
      THROW("Spot light doesn't have spot ");
    }
    if (model_light.Get("spot").Has("innerConeAngle")) {
      inner_cone_angle_ = static_cast<f32>(
          model_light.Get("spot").Get("innerConeAngle").Get<f64>());
    }

    if (model_light.Get("spot").Has("outerConeAngle")) {
      outer_cone_angle_ = static_cast<f32>(
          model_light.Get("spot").Get("outerConeAngle").Get<f64>());
    }
  }
}

std::type_index Light::GetType() { return typeid(Light); }

}  // namespace sg

}  // namespace luka
