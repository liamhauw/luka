/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/
#include "resource/asset/asset.h"

#include <string>

#include "context.h"

namespace luka {

Asset::Asset() : config_{gContext.config} {  }

void Asset::Tick() {
  const std::string& model_file_path{config_->GetModelFilePath().string()};
  if (!gltf_) {
    gltf_ = std::make_unique<Gltf>(model_file_path);
  }
}

}  // namespace luka
