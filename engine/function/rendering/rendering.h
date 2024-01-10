// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/pipeline.h"
#include "function/rendering/rd_context.h"
#include "function/window/window.h"

namespace luka {

class Rendering {
 public:
  Rendering(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu);
  ~Rendering();

  void Tick();

 private:
  void CreateContext();
  void CreatePipeline();

  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;

  std::unique_ptr<rd::Context> context_;
  std::unique_ptr<rd::Pipeline> pipeline_;
};

}  // namespace luka
