// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/camera.h"
#include "function/rendering/gpu.h"

namespace luka {

class Window;

class Rendering {
 public:
  Rendering();
  ~Rendering();

  void Tick();

 private:
  std::unique_ptr<Gpu> gpu_;
  std::unique_ptr<Camera> camera_;
};

}  // namespace luka
