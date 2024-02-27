// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/frame.h"

#include "core/log.h"

namespace luka {

namespace rd {

Frame::Frame(std::shared_ptr<Gpu> gpu) : gpu_{gpu} {
  CreateSyncObjects();
  CreateCommandObjects();
  CreateDescriptorObjects();
}
u32 Frame::AddImages(std::vector<gpu::Image>&& images) {
  u32 images_index{static_cast<u32>(images_.size())};
  images_.push_back(std::move(images));
  return images_index;
}

u32 Frame::AddImageViews(std::vector<vk::raii::ImageView>&& image_views) {
  u32 image_views_index{static_cast<u32>(image_views_.size())};
  image_views_.push_back(std::move(image_views));
  return image_views_index;
}

u32 Frame::AddFramebuffer(vk::raii::Framebuffer&& framebuffer) {
  u32 framebuffer_index{static_cast<u32>(framebuffers_.size())};
  framebuffers_.push_back(std::move(framebuffer));
  return framebuffer_index;
}

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
  render_finished_semaphore_ = gpu_->CreateSemaphoreLuka(semaphore_ci);

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
