// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

namespace luka::gpu {

class Buffer {
 public:
  Buffer() = default;
  Buffer(const Buffer&) = delete;
  Buffer(Buffer&& rhs) noexcept;
  Buffer(const VmaAllocator& allocator, const vk::BufferCreateInfo& buffer_ci,
         bool staging = false);

  ~Buffer();

  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&& rhs) noexcept;
  const vk::Buffer& operator*() const noexcept;

  void Clear() noexcept;

  void* Map();
  void Unmap();

 private:
  VmaAllocator allocator_{};
  vk::Buffer buffer_{nullptr};
  VmaAllocation allocation_{};
  void* mapped_data_{};
};

}  // namespace luka::gpu
