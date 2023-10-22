/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "function/rendering/dynamic_buffer_ring.h"

#include "core/util.h"
#include "function/rendering/common.h"

namespace luka {

Ring::Ring(uint32_t total_size) : total_size_{total_size} {}

RingWithTab::RingWithTab(uint32_t mem_total_size, uint32_t back_buffer_count)
    : mem_{mem_total_size}, back_buffer_count_{back_buffer_count} {}

DynamicBufferRing::DynamicBufferRing(
    const vk::raii::PhysicalDevice& physical_device,
    const vk::raii::Device& device, uint32_t mem_total_size,
    uint32_t back_buffer_count)
    : mem_total_size_{AlignUp(mem_total_size, 256u)},
      mem_{back_buffer_count, mem_total_size_} {
  vk::BufferCreateInfo buffer_ci{{},
                                 mem_total_size_,
                                 vk::BufferUsageFlagBits::eUniformBuffer |
                                     vk::BufferUsageFlagBits::eIndexBuffer |
                                     vk::BufferUsageFlagBits::eVertexBuffer,
                                 vk::SharingMode::eExclusive,
                                 {}};
  buffer_ = vk::raii::Buffer{device, buffer_ci};

  vk::MemoryRequirements mem_requirements{buffer_.getMemoryRequirements()};

  device_memory_ = vk::raii::DeviceMemory{
      AllocateDeviceMemory(physical_device, device, mem_requirements,
                           vk::MemoryPropertyFlagBits::eHostVisible |
                               vk::MemoryPropertyFlagBits::eHostCoherent)};

  buffer_.bindMemory(*(device_memory_), 0);

  data_ = device_memory_.mapMemory(0, mem_requirements.size);
}

}  // namespace luka
