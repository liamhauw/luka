// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/frame.h"

#include "core/log.h"

namespace luka {

namespace rd {

Frame::Frame(std::shared_ptr<Gpu> gpu, vk::Image swapchain_image,
             vk::ImageViewCreateInfo& swapchain_image_view_ci,
             const vk::ImageCreateInfo& depth_image_ci,
             vk::ImageViewCreateInfo depth_image_view_ci)
    : gpu_{gpu},
      target_{gpu_, swapchain_image, swapchain_image_view_ci, depth_image_ci,
              depth_image_view_ci} {
  CreateSyncObjects();
  CreateCommandObjects();
  CreateDescriptorObjects();
}

const Target& Frame::GetTarget() { return target_; }

const vk::raii::Semaphore& Frame::GetRenderFinishedSemphore() const {
  return render_finished_semaphore_;
}

const vk::raii::Fence& Frame::GetCommandFinishedFence() const {
  return command_finished_fence_;
}

const vk::raii::CommandBuffer& Frame::GetActiveCommandBuffer() {
  return command_buffers_[0];
}

const vk::raii::Framebuffer& Frame::GetFramebuffer(u32 i) const {
  return framebuffers_[i];
}

void Frame::CreateSyncObjects() {
  vk::SemaphoreCreateInfo semaphore_ci;
  render_finished_semaphore_ = gpu_->CreateSemaphore0(semaphore_ci);

  vk::FenceCreateInfo fence_ci{vk::FenceCreateFlagBits::eSignaled};
  command_finished_fence_ = gpu_->CreateFence(fence_ci);
}

void Frame::CreateCommandObjects() {
  vk::CommandPoolCreateInfo command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      gpu_->GetGraphicsQueueIndex()};

  command_pool_ = gpu_->CreateCommandPool(command_pool_ci);

  vk::CommandBufferAllocateInfo command_buffer_ai{
      *command_pool_, vk::CommandBufferLevel::ePrimary, 1};

  command_buffers_ = gpu_->AllocateCommandBuffers(command_buffer_ai);
}

void Frame::CreateDescriptorObjects() {}

}  // namespace rd

}  // namespace luka
