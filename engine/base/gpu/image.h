// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

namespace luka::gpu {
class Image {
 public:
  Image() = default;
  Image(const Image&) = delete;
  Image(Image&& rhs) noexcept;
  Image(const VmaAllocator& allocator, const vk::ImageCreateInfo& image_ci);
  explicit Image(vk::Image image);

  ~Image();

  Image& operator=(const Image&) = delete;
  Image& operator=(Image&& rhs) noexcept;
  const vk::Image& operator*() const noexcept;

  void Clear() noexcept;

 private:
  VmaAllocator allocator_{nullptr};
  vk::Image image_{nullptr};
  VmaAllocation allocation_{nullptr};
};
}  // namespace luka::gpu

// namespace  luka
