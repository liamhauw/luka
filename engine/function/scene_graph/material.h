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

class Texture;

enum class AlphaMode { kOpaque, kMask, kBlend };

class Material : public Component {
 public:
  Material(std::array<sg::Texture*, 5>&& textures,
           glm::vec4&& base_color_factor, f32 metallic_factor,
           f32 roughness_factor, f32 scale, f32 strength,
           glm::vec3&& emissive_factor, AlphaMode alpha_mode, f32 alpha_cutoff,
           bool double_sided, const std::string& name = {});
  virtual ~Material() = default;
  std::type_index GetType() override;

 private:
  std::array<sg::Texture*, 5> textures_;
  glm::vec4 base_color_factor_;
  f32 metallic_factor_;
  f32 roughness_factor_;
  f32 scale_;
  f32 strength_;
  glm::vec3 emissive_factor_;
  AlphaMode alpha_mode_;
  f32 alpha_cutoff_;
  bool double_sided_;
};

}  // namespace sg

}  // namespace luka