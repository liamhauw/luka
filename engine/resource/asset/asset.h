/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Asset header file.
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