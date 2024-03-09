// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/gpu/gpu.h"
#include "resource/scene_graph/component.h"

namespace luka {

namespace sg {

class Sampler : public Component {
 public:
  Sampler(vk::raii::Sampler&& sampler,const std::string& name = {});

  Sampler(std::shared_ptr<Gpu> gpu, const tinygltf::Sampler& model_sampler);

  virtual ~Sampler() = default;
  std::type_index GetType() override;

  const vk::raii::Sampler& GetSampler() const;

 private:
  vk::raii::Sampler sampler_{nullptr};
};

}  // namespace sg

}  // namespace luka
