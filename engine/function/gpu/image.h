// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

namespace luka {

namespace gpu {
class Image {
 public:
  Image() = delete;
  Image(std::nullptr_t) {}
  Image(const VmaAllocator& allocator, const vk::ImageCreateInfo& image_ci);
  ~Image();
  Image(const Image&) = delete;
  Image(Image&& rhs) noexcept;
  Image& operator=(const Image&) = delete;
  Image& operator=(Image&& rhs) noexcept;
  const vk::Image& operator*() const noexcept;
  void Clear() noexcept;

 private:
  VmaAllocator allocator_{nullptr};
  vk::Image image_{nullptr};
  VmaAllocation allocation_{nullptr};
};
}  // namespace gpu

}  // namespace  luka
