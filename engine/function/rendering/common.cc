/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "function/rendering/common.h"

#include "core/log.h"

namespace luka {

vk::raii::DeviceMemory AllocateDeviceMemory(
    const vk::raii::PhysicalDevice& physical_device,
    const vk::raii::Device& device,
    const vk::MemoryRequirements& memory_requirements,
    vk::MemoryPropertyFlags memory_properties_flags) {
  vk::PhysicalDeviceMemoryProperties memory_properties{
      physical_device.getMemoryProperties()};

  uint32_t memory_type_bits{memory_requirements.memoryTypeBits};

  uint32_t type_index{static_cast<uint32_t>(~0)};
  for (uint32_t i{0}; i < memory_properties.memoryTypeCount; i++) {
    if ((memory_type_bits & 1) &&
        ((memory_properties.memoryTypes[i].propertyFlags &
          memory_properties_flags) == memory_properties_flags)) {
      type_index = i;
      break;
    }
    memory_type_bits >>= 1;
  }
  if (type_index == static_cast<uint32_t>(~0)) {
    THROW("Fail to find required memory.");
  }

  vk::MemoryAllocateInfo memory_allocate_info{memory_requirements.size,
                                              type_index};

  return vk::raii::DeviceMemory{device, memory_allocate_info};
}

}  // namespace luka
