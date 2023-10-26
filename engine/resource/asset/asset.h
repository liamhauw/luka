/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <memory>

#include "resource/asset/gltf.h"

namespace luka {

class Config;

class Asset {
 public:
  void Tick();

 private:
  std::unique_ptr<Gltf> gltf_;
};

}  // namespace luka