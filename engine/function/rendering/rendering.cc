/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering source file.
*/

#include "function/rendering/rendering.h"

namespace luka {

Rendering::Rendering() {
  gpu_device_ = new Gpu{};
}

void Rendering::Tick() {}

void Rendering::Terminate() {
  delete gpu_device_;
}

}  // namespace luka
