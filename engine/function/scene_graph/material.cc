// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/material.h"

namespace luka {

namespace sg {

Material::Material(std::array<sg::Texture*, 5>&& textures,
                   glm::vec4&& base_color_factor, f32 metallic_factor,
                   f32 roughness_factor, f32 scale, f32 strength,
                   glm::vec3&& emissive_factor, AlphaMode alpha_mode,
                   f32 alpha_cutoff, bool double_sided, const std::string& name)
    : Component{name},
      textures_{textures},
      base_color_factor_{base_color_factor},
      metallic_factor_{metallic_factor},
      roughness_factor_{roughness_factor},
      scale_{scale},
      strength_{strength},
      emissive_factor_{emissive_factor},
      alpha_mode_{alpha_mode},
      alpha_cutoff_{alpha_cutoff},
      double_sided_{double_sided} {}

std::type_index Material::GetType() {
  return typeid(Material);
}

}  // namespace sg

}  // namespace luka
