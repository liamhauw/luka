/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/
#include "resource/asset/asset.h"

#include <string>

#include "context.h"

namespace luka {

void Asset::Tick() {
  if (gContext.load) {
    gltf_.reset();
    const std::string& model_file_path{
        gContext.config->GetModelFilePath().string()};
    gltf_ = std::make_unique<Gltf>(model_file_path);
  }
}

}  // namespace luka
