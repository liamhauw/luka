/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <memory>

#include "function/rendering/gpu.h"

namespace luka {

class Window;

class Rendering {
 public:
  Rendering();
  ~Rendering();

  void Tick();

 private:
  std::shared_ptr<Window> window_;

  std::unique_ptr<Gpu> gpu_;
};

}  // namespace luka
