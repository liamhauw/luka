// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/texture.h"

namespace luka {

namespace ast::sc {

Texture::Texture(const Image* image, const Sampler* sampler,
                 const std::string& name)
    : Component{name}, image_{image}, sampler_{sampler} {}

Texture::Texture(const std::vector<Image*>& image_components,
                 const std::vector<Sampler*>& sampler_components,
                 const tinygltf::Texture& tinygltf_texture)
    : Component{tinygltf_texture.name} {
  if (tinygltf_texture.source != -1) {
    image_ = image_components[tinygltf_texture.source];
  } else {
    image_ = image_components.back();
  }

  if (tinygltf_texture.sampler != -1) {
    sampler_ = sampler_components[tinygltf_texture.sampler];
  } else {
    sampler_ = sampler_components.back();
  }
}

std::type_index Texture::GetType() { return typeid(Texture); }

const Image* Texture::GetImage() const { return image_; }

const Sampler* Texture::GetSampler() const { return sampler_; }

}  // namespace ast::sc

}  // namespace luka