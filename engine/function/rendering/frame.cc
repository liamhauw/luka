// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/frame.h"

namespace luka {

namespace rd {

Frame::Frame(std::unique_ptr<Target>&& target) : target_{std::move(target)} {}

}  // namespace rd

}  // namespace luka
