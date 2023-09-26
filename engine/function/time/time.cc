/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Time source file.
*/

#include "function/time/time.h"

namespace luka {

void Time::Tick() {
  using namespace std::chrono;
  auto now{high_resolution_clock::now()};
  auto diff{now - last_};
  delta_time_ = duration<double>{diff}.count();
  last_ = now;
}

double Time::GetDeltaTime() const { return delta_time_; }

}  // namespace luka
