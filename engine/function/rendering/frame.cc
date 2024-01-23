// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/frame.h"

namespace luka {

namespace rd {

Frame::Frame(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
             vk::ImageViewCreateInfo& swapchain_image_view_ci,
             const vk::ImageCreateInfo& depth_image_ci,
             vk::ImageViewCreateInfo depth_image_view_ci)
    : gpu_{gpu},
      target_{gpu_, swapchain_image, swapchain_image_view_ci, depth_image_ci,
              depth_image_view_ci} {
  CreateCommandObjects();
  CreateSyncObjects();
  CreateDescriptorObjects();
}

const Target& Frame::GetTarget() {
  return target_;
}

void Frame::CreateCommandObjects() {
  vk::CommandPoolCreateInfo command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      gpu_->GetGraphicsQueueIndex()};

  command_pool_ = gpu_->CreateCommandPool(command_pool_ci);

  vk::CommandBufferAllocateInfo command_buffer_ai{
      *command_pool_, vk::CommandBufferLevel::ePrimary, 8};

  command_buffers_ = gpu_->AllocateCommandBuffers(command_buffer_ai);
}

void Frame::CreateSyncObjects() {
  vk::FenceCreateInfo fence_ci{vk::FenceCreateFlagBits::eSignaled};
  fence_ = gpu_->CreateFence(fence_ci);

  vk::SemaphoreCreateInfo semaphore_ci;
  image_available_semaphore_ = gpu_->CreateSemaphore0(semaphore_ci);
  render_finished_semaphore_ = gpu_->CreateSemaphore0(semaphore_ci);
}

void Frame::CreateDescriptorObjects() {}

}  // namespace rd

}  // namespace luka
