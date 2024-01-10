// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/target.h"

namespace luka {

namespace rd {

class Frame {
 public:
  Frame(std::unique_ptr<Target>&& target);

 private:
  std::unique_ptr<Target> target_;
};

}  // namespace rd

}  // namespace luka
