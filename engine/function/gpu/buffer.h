// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

namespace luka {

class Buffer {
 public:
  Buffer() = delete;
  Buffer(std::nullptr_t) {}
  Buffer(const vk::raii::Device& device, const VmaAllocator& allocator,
         const vk::BufferCreateInfo& buffer_ci, bool staging = false,
         const std::string& name = {});
  ~Buffer();
  Buffer(const Buffer&) = delete;
  Buffer(Buffer&& rhs) noexcept;
  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&& rhs) noexcept;

  const vk::Buffer& operator*() const noexcept;

  const VmaAllocation& GetAllocation() const;

 private:
  VmaAllocator allocator_;
  vk::Buffer buffer_{nullptr};
  VmaAllocation allocation_{nullptr};
};

}  // namespace luka
