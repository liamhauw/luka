// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/gpu/image.h"
#include "function/rendering/target.h"

namespace luka {

namespace rd {

class Frame {
 public:
  Frame(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
        vk::ImageViewCreateInfo& swapchain_image_view_ci,
        const vk::ImageCreateInfo& depth_image_ci,
        vk::ImageViewCreateInfo depth_image_view_ci);

  const Target& GetTarget();

 private:
  void CreateCommandObjects();
  void CreateSyncObjects();
  void CreateDescriptorObjects();

  std::shared_ptr<Gpu> gpu_;

  Target target_;

  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::CommandBuffers command_buffers_{nullptr};

  vk::raii::Fence fence_{nullptr};
  vk::raii::Semaphore image_available_semaphore_{nullptr};
  vk::raii::Semaphore render_finished_semaphore_{nullptr};
};

}  // namespace rd

}  // namespace luka
