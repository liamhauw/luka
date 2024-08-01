// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/framework/compute_job.h"

namespace luka::fw {
ComputeJob::ComputeJob() {
  CreatePipeline();
  CreateDescriptorSets();
}

}  // namespace luka::fw