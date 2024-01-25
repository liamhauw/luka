// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/target.h"

namespace luka {

namespace rd {

Target::Target(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
               vk::ImageViewCreateInfo& swapchain_image_view_ci,
               const vk::ImageCreateInfo& depth_image_ci,
               vk::ImageViewCreateInfo depth_image_view_ci) {
  gpu::Image swapchain_image_gpu{swapchain_image};
  images_.push_back(std::move(swapchain_image_gpu));

  swapchain_image_view_ci.image = *(images_.back());
  vk::raii::ImageView swapchain_image_view{
      gpu->CreateImageView(swapchain_image_view_ci, "swapchain")};
  image_views_.push_back(std::move(swapchain_image_view));

  gpu::Image depth_image{gpu->CreateImage(depth_image_ci, "depth")};
  images_.push_back(std::move(depth_image));

  depth_image_view_ci.image = *(images_.back());
  vk::raii::ImageView depth_image_view{
      gpu->CreateImageView(depth_image_view_ci, "depth")};
  image_views_.push_back(std::move(depth_image_view));
}

}  // namespace rd

}  // namespace luka
