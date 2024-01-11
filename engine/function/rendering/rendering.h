// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/pipeline.h"
#include "function/rendering/context.h"
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

  rd::Context context_;
  rd::Pipeline pipeline_;
};

}  // namespace luka
