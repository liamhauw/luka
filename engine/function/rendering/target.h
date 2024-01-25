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
  Target(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
         vk::ImageViewCreateInfo& swapchain_image_view_ci,
         const vk::ImageCreateInfo& depth_image_ci,
         vk::ImageViewCreateInfo depth_image_view_ci);

 private:
  std::vector<gpu::Image> images_;
  std::vector<vk::raii::ImageView> image_views_;
};

}  // namespace rd

}  // namespace luka
