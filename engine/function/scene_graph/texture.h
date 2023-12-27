// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Image;
class Sampler;

class Texture : public Component {
 public:
  Texture(Image* image, Sampler* sampler, const std::string& name = {});
  virtual ~Texture() = default;
  std::type_index GetType() override;

 private:
  Image* image_{nullptr};
  Sampler* sampler_{nullptr};
};

}  // namespace sg

}  // namespace luka