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

class MemoryAllocator {};

class VmaMemoryAllocator : public MemoryAllocator {};

class ResourceManager {
 public:
  ResourceManager() = default;
  virtual ~ResourceManager() = default;
};

class VmaResourceManager : public ResourceManager {
 public:
  VmaResourceManager(const vk::raii::Instance& instance,
                     const vk::raii::PhysicalDevice& physical_device,
                     const vk::raii::Device& device);
  ~VmaResourceManager() override;

 private:
  VmaAllocator vma_allocator_;
};

}  // namespace luka
