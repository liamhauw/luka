// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-swapchain_info.format off
#include "platform/pch.h"
// clang-swapchain_info.format on

#include "function/rendering/rendering.h"

namespace luka {

Rendering::Rendering(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu)
    : window_{window}, gpu_{gpu}, context_{window_, gpu_} {}

Rendering::~Rendering() {}

void Rendering::Tick() {}

}  // namespace luka
