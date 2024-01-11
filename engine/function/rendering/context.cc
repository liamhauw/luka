// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/context.h"

#include "function/gpu/gpu.h"

namespace luka {

namespace rd {

Context::Context(std::shared_ptr<Gpu> gpu, SwapchainInfo swapchain_info,
                 vk::raii::SwapchainKHR&& swapchain,
                 std::vector<std::unique_ptr<Frame>>&& frames)
    : gpu_{gpu},
      swapchain_info_{swapchain_info},
      swapchain_{std::move(swapchain)},
      frames_{std::move(frames)} {}

}  // namespace rd

}  // namespace luka
