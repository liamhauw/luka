/*
  SPDX license identifier: MIT.
  Copyright (C) 2023 Liam Hauw.
*/

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"

namespace luka {

class FunctionUI {
 public:
  FunctionUI();
  ~FunctionUI();

  void Tick();

 private:
  std::shared_ptr<Gpu> gpu_;

  std::array<float, 4> image_data_{1.0f, 1.0f, 0.0f, 1.0f};
  Image image_{nullptr};
  vk::DescriptorSet descriptor_set_{nullptr};
};

}  // namespace luka
