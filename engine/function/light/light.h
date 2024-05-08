// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

constexpr u32 gMaxPunctualLightCount = 8;

enum class PunctualLightType { kDirectional, kPoint, kSpot };

class Light {
 public:
  Light() = default;

  void Tick();

 private:
};

}  // namespace luka