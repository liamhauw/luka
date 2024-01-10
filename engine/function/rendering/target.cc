// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/target.h"

namespace luka {

namespace rd {

Target::Target(vk::Image swapchain_image,
         vk::raii::ImageView&& swapchain_image_view,
         gpu::Image&& depth_image,
         vk::raii::ImageView&& depth_image_view)
    : swapchain_image_{swapchain_image},
      swapchain_image_view_{std::move(swapchain_image_view)},
      depth_image_{std::move(depth_image)},
      depth_image_view_{std::move(depth_image_view)} {}

}  // namespace rd

}  // namespace luka
