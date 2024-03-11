// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/component.h"
#include "resource/asset/scene_component/image.h"
#include "resource/asset/scene_component/sampler.h"

namespace luka {

namespace ast::sc {

class Texture : public Component {
 public:
  Texture(Image* image, Sampler* sampler, const std::string& name = {});

  Texture(const std::vector<Image*>& image_components,
          const std::vector<Sampler*>& sampler_components,
          const tinygltf::Texture& tinygltf_texture);

  virtual ~Texture() = default;
  std::type_index GetType() override;

  Image* GetImage();
  Sampler* GetSampler();

 private:
  Image* image_;
  Sampler* sampler_;
};

}  // namespace ast::sc

}  // namespace luka