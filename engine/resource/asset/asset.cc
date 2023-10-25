/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/
#include "resource/asset/asset.h"

#include <string>

#include "context.h"

namespace luka {

Asset::Asset() : config_{gContext.config} {
  gltf_ = std::make_unique<Gltf>(config_->GetModelFilePath().string());
}

void Asset::Tick() {}

}  // namespace luka
