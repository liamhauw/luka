/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Asset source file.
*/

#include "resource/asset/asset.h"
#include "context.h"
#include <filesystem>

namespace luka {

Asset::Asset() {
  std::filesystem::path model_file_path{gContext.config->GetModelFilePath()};

  
}

void Asset::Tick() {}

}  // namespace luka
