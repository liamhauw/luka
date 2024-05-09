// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/light.h"

#include "core/log.h"

namespace luka {

namespace ast {

Light::Light(const std::filesystem::path& light_path) {
  std::ifstream light_file{light_path.string()};
  if (!light_file) {
    THROW("Fail to load config file {}", light_path.string());
  }
  json_ = json::parse(light_file);
}

}  // namespace ast

}  // namespace luka