// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/time/time.h"

namespace luka {

void Time::Tick() {
  auto now{std::chrono::high_resolution_clock::now()};
  auto diff{now - last_};
  delta_time_ = std::chrono::duration<f64>{diff}.count();
  last_ = now;
}

f64 Time::GetDeltaTime() const { return delta_time_; }

}  // namespace luka
