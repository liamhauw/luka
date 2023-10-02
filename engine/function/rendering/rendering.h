/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering header file.
*/

#pragma once

#include <memory>
#include "function/rendering/gpu.h"

namespace luka {

class Rendering {
 public:
  Rendering();

  void Tick();

  void Terminate();

 private:
  Gpu* gpu_device_;
};

}  // namespace luka
