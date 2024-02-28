// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/texture.h"

namespace luka {

namespace sg {

Texture::Texture(Image* image, Sampler* sampler, const std::string& name)
    : Component{name}, image_{image}, sampler_{sampler} {}

Texture::Texture(const std::vector<Image*>& image_components,
                 const std::vector<Sampler*>& sampler_components,
                 const tinygltf::Texture& model_texture)
    : Component{model_texture.name} {
  if (model_texture.source != -1) {
    image_ = image_components[model_texture.source];
  } else {
    image_ = image_components.back();
  }

  if (model_texture.sampler != -1) {
    sampler_ = sampler_components[model_texture.sampler];
  } else {
    sampler_ = sampler_components.back();
  }
}

std::type_index Texture::GetType() { return typeid(Texture); }

Image* Texture::GetImage() { return image_; }

Sampler* Texture::GetSampler() { return sampler_; }

}  // namespace sg

}  // namespace luka