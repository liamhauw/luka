/*
  SPDX license identifier: MIT.

  Copyright (C) 2023 Liam Hauw.

  Time header file.
*/

#pragma once

#include <chrono>

namespace luka {

class Time {
 public:
  void Tick();
  double GetDeltaTime() const;

 private:
  double delta_time_{0.0};
  std::chrono::high_resolution_clock::time_point last_{
      std::chrono::high_resolution_clock::now()};
};

}  // namespace luka
