// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/gpu/image.h"

namespace luka {

namespace rd {

class Target {
 public:
  Target(vk::Image swapchain_image, vk::raii::ImageView&& swapchain_image_view,
         gpu::Image&& depth_image, vk::raii::ImageView&& depth_image_view);

  Target(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
         vk::ImageViewCreateInfo& swapchain_image_view_ci,
         const vk::ImageCreateInfo& depth_image_ci,
         vk::ImageViewCreateInfo depth_image_view_ci);

 private:
  vk::Image swapchain_image_{nullptr};
  vk::raii::ImageView swapchain_image_view_{nullptr};
  gpu::Image depth_image_{nullptr};
  vk::raii::ImageView depth_image_view_{nullptr};
};

}  // namespace rd

}  // namespace luka
