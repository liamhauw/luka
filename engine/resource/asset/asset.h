/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <stb_image.h>
#include <tiny_gltf.h>

namespace luka {

class Asset {
 public:
  Asset();

  void Tick();
};

}  // namespace luka