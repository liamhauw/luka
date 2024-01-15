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
               gpu::Image&& depth_image, vk::raii::ImageView&& depth_image_view)
    : swapchain_image_{swapchain_image},
      swapchain_image_view_{std::move(swapchain_image_view)},
      depth_image_{std::move(depth_image)},
      depth_image_view_{std::move(depth_image_view)} {}

Target::Target(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
               vk::ImageViewCreateInfo& swapchain_image_view_ci,
               const vk::ImageCreateInfo& depth_image_ci,
               vk::ImageViewCreateInfo depth_image_view_ci)
    : swapchain_image_{swapchain_image},
      depth_image_{gpu->CreateImage(depth_image_ci, "depth")} {
  swapchain_image_view_ci.image = swapchain_image;
  swapchain_image_view_ =
      gpu->CreateImageView(swapchain_image_view_ci, "swapchain");
  depth_image_view_ci.image = *depth_image_;
  depth_image_view_ = gpu->CreateImageView(depth_image_view_ci, "depth");
}

}  // namespace rd

}  // namespace luka
