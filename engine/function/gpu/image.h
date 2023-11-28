// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

namespace luka {

class Image {
 public:
  Image() = delete;
  Image(std::nullptr_t) {}
  Image(const vk::raii::Device& device, const VmaAllocator& allocator,
        const vk::ImageCreateInfo& image_ci, const std::string& name = {});
  ~Image();
  Image(const Image&) = delete;
  Image(Image&& rhs) noexcept;
  Image& operator=(const Image&) = delete;
  Image& operator=(Image&& rhs) noexcept;

  const vk::Image& operator*() const noexcept;
  const vk::DescriptorImageInfo& GetDescriptorImageInfo() const;

 private:
  VmaAllocator allocator_;
  vk::Image image_{nullptr};
  VmaAllocation allocation_{nullptr};
  vk::raii::ImageView image_view_{nullptr};
  vk::raii::Sampler sampler_{nullptr};
  vk::DescriptorImageInfo descriptor_image_info_;
};

}  // namespace  luka
