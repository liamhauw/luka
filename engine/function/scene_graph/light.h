// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/math.h"
#include "function/scene_graph/component.h"

namespace luka {
namespace sg {

enum class LightType { kNone = -1, kDirectional, kPoint, kSpot, kCount };

struct LightProperty {
  LightType type{LightType::kNone};
  glm::vec3 direction{0.0F, 0.0F, -1.0F};
  glm::vec3 color{1.0F, 1.0F, 1.0F};
  f32 intensity{1.0F};
  f32 range{0.0F};
  f32 inner_cone_angle{0.0F};
  f32 outer_cone_angle{glm::quarter_pi<f32>()};
};

class Light : public Component {
 public:
  Light(const LightProperty& property, const std::string& name = {});
  virtual ~Light() = default;
  std::type_index GetType() override;

 private:
  LightProperty property_;
};

}  // namespace sg

}  // namespace luka
