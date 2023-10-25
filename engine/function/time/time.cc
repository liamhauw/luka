/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Time source file.
*/

#include "function/time/time.h"

namespace luka {

void Time::Tick() {
  auto now{std::chrono::high_resolution_clock::now()};
  auto diff{now - last_};
  delta_time_ = std::chrono::duration<double>{diff}.count();
  last_ = now;
}

double Time::GetDeltaTime() const { return delta_time_; }

}  // namespace luka
