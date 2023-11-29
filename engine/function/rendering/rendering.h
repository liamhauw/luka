// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Gpu;
class FunctionUI;

class Rendering {
 public:
  Rendering();

  void Tick();

 private:
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<FunctionUI> function_ui_;
};

}  // namespace luka
