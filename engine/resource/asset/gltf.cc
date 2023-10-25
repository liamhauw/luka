/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "resource/asset/gltf.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "core/log.h"

namespace luka {

Gltf::Gltf(const std::string& model_file_path) {
  std::ifstream model_file{model_file_path};
  if (!model_file) {
    THROW("Fail to open {}", model_file_path);
  }

  nlohmann::json model_data{nlohmann::json::parse(model_file)};


}

}  // namespace luka
