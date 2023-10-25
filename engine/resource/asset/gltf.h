/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <memory>
#include <string>

namespace luka {

class Gltf {
 public:
  Gltf(const std::string& model_file_path);
};

}  // namespace luka
