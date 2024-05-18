// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "base/gpu/gpu.h"
#include "resource/asset/scene_component/component.h"

namespace luka {

namespace ast::sc {

class Sampler : public Component {
 public:
  Sampler(vk::raii::Sampler&& sampler, const std::string& name = {});

  Sampler(std::shared_ptr<Gpu> gpu, const tinygltf::Sampler& tinygltf_sampler);

  virtual ~Sampler() = default;
  std::type_index GetType() override;

  const vk::raii::Sampler& GetSampler() const;

 private:
  vk::raii::Sampler sampler_{nullptr};
};

}  // namespace ast::sc

}  // namespace luka
