// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/gpu/gpu.h"

namespace luka::fw {
class ComputeJob {
 public:
  ComputeJob();

 private:
  vk::raii::Pipeline pipeline_{nullptr};
  vk::raii::DescriptorSets descriptor_sets_{nullptr};
  u32 group_count_x_{};
  u32 group_count_y_{};
  u32 group_count_z_{};
};

}  // namespace luka::fw