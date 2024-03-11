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

namespace luka {

namespace ast::sc {

enum class LightType { kNone = -1, kDirectional, kPoint, kSpot, kCount };

class Light : public Component {
 public:
  Light(LightType type, glm::vec3 direction, glm::vec3 color, f32 intensity,
        f32 range, f32 inner_cone_angle, f32 outer_cone_angle,
        const std::string& name = {});

  Light(const tinygltf::Value& tinygltf_light);

  virtual ~Light() = default;
  std::type_index GetType() override;

 private:
  LightType type_;
  glm::vec3 direction_;
  glm::vec3 color_;
  f32 intensity_;
  f32 range_;
  f32 inner_cone_angle_;
  f32 outer_cone_angle_;
};

}  // namespace ast::sc

}  // namespace luka
