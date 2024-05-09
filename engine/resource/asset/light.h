// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"

namespace luka {

namespace ast {
constexpr u32 gMaxPunctualLightCount = 8;

enum class PunctualLightType { kDirectional, kPoint, kSpot };

class Light {
 public:
  Light() = default;

  Light(const std::filesystem::path& light_path);

 private:
  json json_;
};
}  // namespace ast

}  // namespace luka