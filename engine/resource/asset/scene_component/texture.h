// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/util.h"
#include "resource/asset/scene_component/component.h"
#include "resource/asset/scene_component/image.h"
#include "resource/asset/scene_component/sampler.h"

namespace luka::ast::sc {

class Texture : public Component {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Texture)

  Texture(const Image* image, const Sampler* sampler,
          const std::string& name = {});
  Texture(const std::vector<Image*>& image_components,
          const std::vector<Sampler*>& sampler_components,
          const tinygltf::Texture& tinygltf_texture);

  ~Texture() override = default;

  std::type_index GetType() override;

  const Image* GetImage() const;
  const Sampler* GetSampler() const;

 private:
  const Image* image_;
  const Sampler* sampler_;
};

}  // namespace luka::ast::sc
