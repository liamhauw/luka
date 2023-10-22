/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace luka {

vk::raii::DeviceMemory AllocateDeviceMemory(
    const vk::raii::PhysicalDevice& physical_device,
    const vk::raii::Device& device,
    const vk::MemoryRequirements& memory_requirements,
    vk::MemoryPropertyFlags memory_properties_flags);

}  // namespace luka
