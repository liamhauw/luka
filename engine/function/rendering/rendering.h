// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/pipeline.h"
#include "function/rendering/rd_context.h"

namespace luka {

class Gpu;

class Rendering {
 public:
  Rendering();
  ~Rendering();

  void Tick();

 private:
  void CreateContext();
  void CreatePipeline();

  std::shared_ptr<Gpu> gpu_;

  std::unique_ptr<rd::Context> context_;
  std::unique_ptr<rd::Pipeline> pipeline_;
};

}  // namespace luka
