// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/math.h"
#include "resource/asset/scene_component/component.h"

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace luka::ast::sc {

enum class LightType { kNone, kDirectional, kPoint, kSpot, kCount };

class Light : public Component {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Light)

  Light(LightType type, glm::vec3 direction, glm::vec3 color, f32 intensity,
        f32 range, f32 inner_cone_angle, f32 outer_cone_angle,
        const std::string& name = {});
  explicit Light(const tinygltf::Value& tinygltf_light);

  ~Light() override = default;

  std::type_index GetType() override;

 private:
  LightType type_{LightType::kNone};
  glm::vec3 direction_{};
  glm::vec3 color_{};
  f32 intensity_{};
  f32 range_{};
  f32 inner_cone_angle_{};
  f32 outer_cone_angle_{};
};

}  // namespace luka::ast::sc
