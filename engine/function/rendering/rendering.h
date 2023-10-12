/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering header file.
*/

#pragma once

#include <memory>

#include "function/rendering/gpu.h"

namespace luka {

class Asset;
class Window;

class Rendering {
 public:
  Rendering();
  ~Rendering();

  void Tick();

 private:
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Window> window_;

  std::unique_ptr<Gpu> gpu_;
};

}  // namespace luka
