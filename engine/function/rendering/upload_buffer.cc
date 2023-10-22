/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "function/rendering/upload_buffer.h"

#include "function/rendering/common.h"

namespace luka {

UploadBuffer::UploadBuffer(const vk::raii::PhysicalDevice& physical_device,
                       const vk::raii::Device& device,
                       uint32_t graphics_queue_index, uint32_t mem_total_size) {
  // Command objects
  command_pool_ =
      vk::raii::CommandPool{device,
                            {vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                             graphics_queue_index}};

  vk::CommandBufferAllocateInfo command_buffer_allocate_info{
      *command_pool_, vk::CommandBufferLevel::ePrimary, 1};
  command_buffer_ = std::move(
      vk::raii::CommandBuffers{device, command_buffer_allocate_info}[0]);

  // Fence.
  fence_ = vk::raii::Fence{device, vk::FenceCreateInfo{}};

  // Buffer.
  vk::BufferCreateInfo buffer_ci{
      {}, mem_total_size, vk::BufferUsageFlagBits::eTransferSrc};
  buffer_ = vk::raii::Buffer{device, buffer_ci};

  vk::MemoryRequirements mem_requirements{buffer_.getMemoryRequirements()};
  device_memory_ = vk::raii::DeviceMemory{
      AllocateDeviceMemory(physical_device, device, mem_requirements,
                           vk::MemoryPropertyFlagBits::eHostVisible)};

  buffer_.bindMemory(*(device_memory_), 0);

  data_begin_ = reinterpret_cast<uint8_t*>(
      device_memory_.mapMemory(0, mem_requirements.size));
  data_cur_ = data_begin_;
  data_end_ = data_begin_ + mem_requirements.size;

  // Begin command buffer.
  command_buffer_.begin({});
}

}  // namespace luka
