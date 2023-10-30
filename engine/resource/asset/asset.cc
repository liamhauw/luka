// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on
#include "resource/asset/asset.h"

#include "context.h"

namespace luka {

void Asset::Tick() {
  if (gContext.load) {
    gltf_.reset();
    gltf_ =
        std::make_unique<Gltf>(gContext.config->GetModelFilePath().string());
  }
}

}  // namespace luka
