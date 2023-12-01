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

  void SetImage(const std::vector<vk::DescriptorSet>& descriptor_set);
  void Render(const vk::raii::CommandBuffer& command_buffer);

 private:
  std::shared_ptr<Gpu> gpu_;

  std::vector<vk::DescriptorSet> descriptor_sets_;
};

}  // namespace luka
