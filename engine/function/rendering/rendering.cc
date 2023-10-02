/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering source file.
*/

#include "function/rendering/rendering.h"

namespace luka {

Rendering::Rendering() {
  gpu_device_ = std::make_unique<Gpu>();
}

void Rendering::Tick() {}

void Rendering::Terminate() {
  gpu_device_.reset();
}

}  // namespace luka
