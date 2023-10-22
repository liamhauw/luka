/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "function/rendering/static_buffer.h"

#include "function/rendering/common.h"

namespace luka {

StaticBuffer::StaticBuffer(const vk::raii::PhysicalDevice& physical_device,
                           const vk::raii::Device& device,
                           uint32_t mem_total_size, bool is_device)
    : mem_total_size_{mem_total_size}, is_device_{is_device} {
  {
    vk::BufferCreateInfo buffer_ci{{},
                                   mem_total_size_,
                                   vk::BufferUsageFlagBits::eIndexBuffer |
                                       vk::BufferUsageFlagBits::eVertexBuffer};
    if (is_device_) {
      buffer_ci.usage |= vk::BufferUsageFlagBits::eTransferSrc;
    }
    host_buffer_ = vk::raii::Buffer{device, buffer_ci};

    vk::MemoryRequirements mem_requirements{
        host_buffer_.getMemoryRequirements()};
    host_memory_ = vk::raii::DeviceMemory{
        AllocateDeviceMemory(physical_device, device, mem_requirements,
                             vk::MemoryPropertyFlagBits::eHostVisible |
                                 vk::MemoryPropertyFlagBits::eHostCoherent)};

    host_buffer_.bindMemory(*(host_memory_), 0);

    data_ = host_memory_.mapMemory(0, mem_requirements.size);
  }

  if (is_device_) {
    vk::BufferCreateInfo buffer_ci{{},
                                   mem_total_size_,
                                   vk::BufferUsageFlagBits::eIndexBuffer |
                                       vk::BufferUsageFlagBits::eVertexBuffer |
                                       vk::BufferUsageFlagBits::eTransferDst};
    device_buffer_ = vk::raii::Buffer{device, buffer_ci};

    vk::MemoryRequirements mem_requirements{
        device_buffer_.getMemoryRequirements()};
    device_device_memory_ = vk::raii::DeviceMemory{
        AllocateDeviceMemory(physical_device, device, mem_requirements,
                             vk::MemoryPropertyFlagBits::eDeviceLocal)};

    device_buffer_.bindMemory(*(device_device_memory_), 0);
  }
}

}  // namespace luka
