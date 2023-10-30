// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Time {
 public:
  void Tick();
  f64 GetDeltaTime() const;

 private:
  f64 delta_time_{0.0};
  std::chrono::high_resolution_clock::time_point last_{
      std::chrono::high_resolution_clock::now()};
};

}  // namespace luka
