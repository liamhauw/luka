// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/frame.h"

namespace luka {

namespace rd {

Frame::Frame(Target&& target) : target_{std::move(target)} {}

Frame::Frame(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
             vk::ImageViewCreateInfo& swapchain_image_view_ci,
             const vk::ImageCreateInfo& depth_image_ci,
             vk::ImageViewCreateInfo depth_image_view_ci)
    : target_{gpu, swapchain_image, swapchain_image_view_ci, depth_image_ci,
              depth_image_view_ci} {}

}  // namespace rd

}  // namespace luka
