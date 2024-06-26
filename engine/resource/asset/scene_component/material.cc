// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/material.h"

#include "core/log.h"
#include "core/util.h"

namespace luka::ast::sc {

Material::Material(std::map<std::string, Texture*>&& textures,
                   glm::vec4&& base_color_factor, f32 metallic_factor,
                   f32 roughness_factor, f32 scale, f32 strength,
                   glm::vec3&& emissive_factor, AlphaMode alpha_mode,
                   f32 alpha_cutoff, bool double_sided, const std::string& name)
    : Component{name},
      textures_{std::move(textures)},
      base_color_factor_{base_color_factor},
      metallic_factor_{metallic_factor},
      roughness_factor_{roughness_factor},
      normal_scale_{scale},
      occlusion_strength_{strength},
      emissive_factor_{emissive_factor},
      alpha_mode_{alpha_mode},
      alpha_cutoff_{alpha_cutoff},
      double_sided_{double_sided} {}

Material::Material(const std::vector<Texture*>& texture_components,
                   const tinygltf::Material& tinygltf_material)
    : Component{tinygltf_material.name} {
  // Pbr.
  const tinygltf::PbrMetallicRoughness& metallic_roughness{
      tinygltf_material.pbrMetallicRoughness};

  std::vector<f32> base_color_factor_fv{
      D2FVector(metallic_roughness.baseColorFactor)};
  base_color_factor_ = glm::make_vec4(base_color_factor_fv.data());

  ast::sc::Texture* base_color_texture{};
  if (metallic_roughness.baseColorTexture.index != -1) {
    base_color_texture =
        texture_components[metallic_roughness.baseColorTexture.index];
    textures_.insert(std::make_pair("base_color_texture", base_color_texture));
  }

  metallic_factor_ = static_cast<f32>(metallic_roughness.metallicFactor);

  roughness_factor_ = static_cast<f32>(metallic_roughness.roughnessFactor);

  ast::sc::Texture* metallic_roughness_texture{};
  if (metallic_roughness.metallicRoughnessTexture.index != -1) {
    metallic_roughness_texture =
        texture_components[metallic_roughness.metallicRoughnessTexture.index];
    textures_.insert(std::make_pair("metallic_roughness_texture",
                                    metallic_roughness_texture));
  }

  // Normal.
  const tinygltf::NormalTextureInfo& normal{tinygltf_material.normalTexture};

  ast::sc::Texture* normal_texture{};
  if (normal.index != -1) {
    normal_texture = texture_components[normal.index];
    textures_.insert(std::make_pair("normal_texture", normal_texture));
  }
  normal_scale_ = static_cast<f32>(normal.scale);

  // Occlusion.
  const tinygltf::OcclusionTextureInfo& occlusion{
      tinygltf_material.occlusionTexture};

  ast::sc::Texture* occlusion_texture{};
  if (occlusion.index != -1) {
    occlusion_texture = texture_components[occlusion.index];
    textures_.insert(std::make_pair("occlusion_texture", occlusion_texture));
  }

  occlusion_strength_ = static_cast<f32>(occlusion.strength);

  // Emissive.
  std::vector<f32> emissive_factor_fv{
      D2FVector(tinygltf_material.emissiveFactor)};
  emissive_factor_ = glm::make_vec3(emissive_factor_fv.data());

  ast::sc::Texture* emissive_texture{};
  if (tinygltf_material.emissiveTexture.index != -1) {
    emissive_texture =
        texture_components[tinygltf_material.emissiveTexture.index];
    textures_.insert(std::make_pair("emissive_texture", emissive_texture));
  }

  // Others.
  const std::string& alpha_mode_str{tinygltf_material.alphaMode};
  if (alpha_mode_str == "OPAQUE") {
    alpha_mode_ = ast::sc::AlphaMode::kOpaque;
  } else if (alpha_mode_str == "MASK") {
    alpha_mode_ = ast::sc::AlphaMode::kMask;
  } else if (alpha_mode_str == "BLEND") {
    alpha_mode_ = ast::sc::AlphaMode::kBlend;
  } else {
    THROW("Unsupported alpha mode");
  }

  alpha_cutoff_ = static_cast<f32>(tinygltf_material.alphaCutoff);

  double_sided_ = tinygltf_material.doubleSided;
}

std::type_index Material::GetType() { return typeid(Material); }

const std::map<std::string, Texture*>& Material::GetTextures() const {
  return textures_;
}

const glm::vec4& Material::GetBaseColorFactor() const {
  return base_color_factor_;
}

f32 Material::GetMetallicFactor() const { return metallic_factor_; }

f32 Material::GetRoughnessFactor() const { return roughness_factor_; }

f32 Material::GetNormalScale() const { return normal_scale_; }

f32 Material::GetOcclusionStrength() const { return occlusion_strength_; }

const glm::vec3& Material::GetEmissiveFactor() const {
  return emissive_factor_;
}

AlphaMode Material::GetAlphaMode() const { return alpha_mode_; }

f32 Material::GetAlphaCutoff() const { return alpha_cutoff_; }

bool Material::GetDoubleSided() const { return double_sided_; }

}  // namespace luka::ast::sc
