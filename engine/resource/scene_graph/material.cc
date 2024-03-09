// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/scene_graph/material.h"

#include "core/log.h"
#include "core/util.h"

namespace luka {

namespace sg {

Material::Material(std::map<std::string, Texture*>&& textures,
                   glm::vec4&& base_color_factor, f32 metallic_factor,
                   f32 roughness_factor, f32 scale, f32 strength,
                   glm::vec3&& emissive_factor, AlphaMode alpha_mode,
                   f32 alpha_cutoff, bool double_sided, const std::string& name)
    : Component{name},
      textures_{std::move(textures)},
      base_color_factor_{std::move(base_color_factor)},
      metallic_factor_{metallic_factor},
      roughness_factor_{roughness_factor},
      scale_{scale},
      strength_{strength},
      emissive_factor_{std::move(emissive_factor)},
      alpha_mode_{alpha_mode},
      alpha_cutoff_{alpha_cutoff},
      double_sided_{double_sided} {}

Material::Material(const std::vector<Texture*> texture_components,
                   const tinygltf::Material& model_material)
    : Component{model_material.name} {
  // Pbr.
  const tinygltf::PbrMetallicRoughness& metallic_roughness{
      model_material.pbrMetallicRoughness};

  std::vector<f32> base_color_factor_fv{
      D2FVector(metallic_roughness.baseColorFactor)};
  base_color_factor_ = glm::make_vec4(base_color_factor_fv.data());

  sg::Texture* base_color_texture;
  if (metallic_roughness.baseColorTexture.index != -1) {
    base_color_texture =
        texture_components[metallic_roughness.baseColorTexture.index];
    textures_.insert(std::make_pair("base_color_texture", base_color_texture));
  }

  metallic_factor_ = static_cast<f32>(metallic_roughness.metallicFactor);

  roughness_factor_ = static_cast<f32>(metallic_roughness.roughnessFactor);

  sg::Texture* metallic_roughness_texture;
  if (metallic_roughness.metallicRoughnessTexture.index != -1) {
    metallic_roughness_texture =
        texture_components[metallic_roughness.metallicRoughnessTexture.index];
    textures_.insert(
        std::make_pair("metallic_roughness_texture", metallic_roughness_texture));
  }

  // Normal.
  const tinygltf::NormalTextureInfo& normal{model_material.normalTexture};

  sg::Texture* normal_texture;
  if (normal.index != -1) {
    normal_texture = texture_components[normal.index];
    textures_.insert(std::make_pair("normal_texture", normal_texture));
  }
  scale_ = static_cast<f32>(normal.scale);

  // Occlusion.
  const tinygltf::OcclusionTextureInfo& occlusion{
      model_material.occlusionTexture};

  sg::Texture* occlusion_texture;
  if (occlusion.index != -1) {
    occlusion_texture = texture_components[occlusion.index];
    textures_.insert(std::make_pair("occlusion_texture", occlusion_texture));
  }

  strength_ = static_cast<f32>(occlusion.strength);

  // Emissive.
  std::vector<f32> emissive_factor_fv{D2FVector(model_material.emissiveFactor)};
  emissive_factor_ = glm::make_vec3(emissive_factor_fv.data());

  sg::Texture* emissive_texture;
  if (model_material.emissiveTexture.index != -1) {
    emissive_texture = texture_components[model_material.emissiveTexture.index];
    textures_.insert(std::make_pair("emissive_texture", emissive_texture));
  }

  // Others.
  const std::string& alpha_mode_str{model_material.alphaMode};
  if (alpha_mode_str == "OPAQUE") {
    alpha_mode_ = sg::AlphaMode::kOpaque;
  } else if (alpha_mode_str == "MASK") {
    alpha_mode_ = sg::AlphaMode::kMask;
  } else if (alpha_mode_str == "BLEND") {
    alpha_mode_ = sg::AlphaMode::kBlend;
  } else {
    THROW("Unsupported alpha mode");
  }

  alpha_cutoff_ = static_cast<f32>(model_material.alphaCutoff);

  double_sided_ = model_material.doubleSided;
}

std::type_index Material::GetType() { return typeid(Material); }

const std::map<std::string, Texture*>& Material::GetTextures() const {
  return textures_;
}

const glm::vec4& Material::GetBaseColorFactor() const {
  return base_color_factor_;
}


}  // namespace sg

}  // namespace luka
