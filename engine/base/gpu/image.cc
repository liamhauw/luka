// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/gpu/image.h"

#include "core/log.h"

namespace luka {

namespace gpu {
Image::Image(const VmaAllocator& allocator, const vk::ImageCreateInfo& image_ci)
    : allocator_{allocator} {
  VkImageCreateInfo vk_image_ci{static_cast<VkImageCreateInfo>(image_ci)};
  VmaAllocationCreateInfo allocation_ci{.usage = VMA_MEMORY_USAGE_AUTO};
  VkImage image;
  vmaCreateImage(allocator_, &vk_image_ci, &allocation_ci, &image, &allocation_,
                 nullptr);
  image_ = image;
}

Image::Image(vk::Image image) : image_{image} {}

Image::Image(Image&& rhs) noexcept
    : allocator_{std::exchange(rhs.allocator_, {})},
      image_{std::exchange(rhs.image_, {})},
      allocation_{std::exchange(rhs.allocation_, {})} {}

Image::~Image() { Clear(); }

Image& Image::operator=(Image&& rhs) noexcept {
  if (this != &rhs) {
    std::swap(allocator_, rhs.allocator_);
    std::swap(image_, rhs.image_);
    std::swap(allocation_, rhs.allocation_);
  }
  return *this;
}

const vk::Image& Image::operator*() const noexcept { return image_; }

void Image::Clear() noexcept {
  if (allocator_ && image_ && allocation_) {
    vmaDestroyImage(allocator_, static_cast<VkImage>(image_), allocation_);
  }
  allocator_ = nullptr;
  image_ = nullptr;
  allocation_ = nullptr;
}
}  // namespace gpu

}  // namespace luka
