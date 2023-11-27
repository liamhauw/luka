// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"

namespace luka {

class Rendering {
 public:
  Rendering();

  void Tick();

 private:
  std::shared_ptr<Gpu> gpu_;

  Image image_{nullptr};
};

}  // namespace luka
