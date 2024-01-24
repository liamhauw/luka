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

  const vk::raii::Semaphore& GetRenderFinishedSemphore() const;
  const vk::raii::Fence& GetCommandFinishedFence() const;

  const vk::raii::CommandBuffer& GetActiveCommandBuffer();

 private:
  void CreateSyncObjects();
  void CreateCommandObjects();
  void CreateDescriptorObjects();

  std::shared_ptr<Gpu> gpu_;

  Target target_;

  vk::raii::Semaphore render_finished_semaphore_{nullptr};
  vk::raii::Fence command_finished_fence_{nullptr};

  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::CommandBuffers command_buffers_{nullptr};

  vk::raii::DescriptorPool normal_descriptor_pool_{nullptr};
  vk::raii::DescriptorSets normal_descriptor_sets_{nullptr};
  vk::raii::DescriptorPool bindless_descriptor_pool_{nullptr};
  vk::raii::DescriptorSet bindless_descriptor_set_{nullptr};
};

}  // namespace rd

}  // namespace luka
