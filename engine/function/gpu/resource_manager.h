/*
  SPDX license identifier: MIT.
  Copyright (C) 2023 Liam Hauw.
*/

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"

namespace luka {

class MemoryAllocator {
 public:
  MemoryAllocator() = default;
  virtual ~MemoryAllocator() = default;
};

class VmaMemoryAllocator : public MemoryAllocator {
 public:
  VmaMemoryAllocator(const vk::raii::Instance& instance,
                     const vk::raii::PhysicalDevice& physical_device,
                     const vk::raii::Device& device);
  ~VmaMemoryAllocator() override;

 private:
  VmaAllocator vma_allocator_;
};

class ResourceManager {
 public:
  ResourceManager() = default;
  virtual ~ResourceManager() = default;

 protected:
  std::unique_ptr<MemoryAllocator> memory_allocator_;
};

class VmaResourceManager : public ResourceManager {
 public:
  VmaResourceManager(const vk::raii::Instance& instance,
                     const vk::raii::PhysicalDevice& physical_device,
                     const vk::raii::Device& device);
  ~VmaResourceManager() override;
};

}  // namespace luka
