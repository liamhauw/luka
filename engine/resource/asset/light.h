// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"
#include "core/math.h"

namespace luka::ast {

enum class PunctualLightType : u32 { kDirectional = 0, kPoint, kSpot };

struct PunctualLight {
  glm::vec3 position;
  u32 type;

  glm::vec3 direction;
  f32 intensity;

  glm::vec3 color;
  f32 range;

  f32 inner_cone_cos;
  f32 outer_cone_cos;
  glm::vec2 padding;
};

class Light {
 public:
  Light() = default;

  explicit Light(const std::filesystem::path& light_path);

  const std::vector<PunctualLight>& GetPunctualLights() const;

 private:
  json json_;

  std::vector<PunctualLight> puntual_lights_;
};
}  // namespace luka::ast
