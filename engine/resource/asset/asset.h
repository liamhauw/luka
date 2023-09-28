/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Asset header file.
*/

#pragma once

#include <tiny_gltf.h>

namespace luka {

class Asset {
 public:
  Asset();

  void Tick();

 private:
  tinygltf::Model model_;
};

}  // namespace luka