// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"
#include "function/scene_graph/image.h"
#include "function/scene_graph/sampler.h"

namespace luka {

namespace sg {

class Texture : public Component {
 public:
  Texture(Image* image, Sampler* sampler, const std::string& name = {});

  Texture(const std::vector<Image*>& image_components,
          const std::vector<Sampler*>& sampler_components,
          const tinygltf::Texture& model_texture);

  virtual ~Texture() = default;
  std::type_index GetType() override;

 private:
  Image* image_;
  Sampler* sampler_;
};

}  // namespace sg

}  // namespace luka