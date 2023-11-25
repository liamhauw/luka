/*
  SPDX license identifier: MIT.
  Copyright (C) 2023 Liam Hauw.
*/

// clang-format off
#include "platform/pch.h"
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

#include "function/gpu/resource_manager.h"

namespace luka {

VmaMemoryAllocator::VmaMemoryAllocator(
    const vk::raii::Instance& instance,
    const vk::raii::PhysicalDevice& physical_device,
    const vk::raii::Device& device) {
  VmaAllocatorCreateInfo allocator_ci{};
  allocator_ci.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  allocator_ci.instance = static_cast<VkInstance>(*instance);
  allocator_ci.physicalDevice = static_cast<VkPhysicalDevice>(*physical_device);
  allocator_ci.device = static_cast<VkDevice>(*device);
  allocator_ci.vulkanApiVersion = VK_API_VERSION_1_3;
  vmaCreateAllocator(&allocator_ci, &vma_allocator_);
}

VmaMemoryAllocator::~VmaMemoryAllocator() {
  vmaDestroyAllocator(vma_allocator_);
}

VmaResourceManager::VmaResourceManager(
    const vk::raii::Instance& instance,
    const vk::raii::PhysicalDevice& physical_device,
    const vk::raii::Device& device) {
  memory_allocator_ =
      std::make_unique<VmaMemoryAllocator>(instance, physical_device, device);
}

VmaResourceManager::~VmaResourceManager() {}

}  // namespace luka
