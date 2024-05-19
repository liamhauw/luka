// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/gpu/buffer.h"

#include "core/log.h"

namespace luka::gpu {

Buffer::Buffer(Buffer&& rhs) noexcept
    : allocator_{std::exchange(rhs.allocator_, {})},
      buffer_{std::exchange(rhs.buffer_, {})},
      allocation_{std::exchange(rhs.allocation_, {})},
      mapped_data_{std::exchange(rhs.mapped_data_, nullptr)} {}

Buffer::Buffer(const VmaAllocator& allocator,
               const vk::BufferCreateInfo& buffer_ci, bool staging)
    : allocator_{allocator} {
  VkBufferCreateInfo vk_buffer_ci{static_cast<VkBufferCreateInfo>(buffer_ci)};
  VmaAllocationCreateInfo allocation_ci{.usage = VMA_MEMORY_USAGE_AUTO};
  if (staging) {
    allocation_ci.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
  }
  VkBuffer buffer{};
  vmaCreateBuffer(allocator_, &vk_buffer_ci, &allocation_ci, &buffer,
                  &allocation_, nullptr);
  buffer_ = buffer;
}

Buffer::~Buffer() {
  Unmap();
  Clear();
}

Buffer& Buffer::operator=(Buffer&& rhs) noexcept {
  if (this != &rhs) {
    std::swap(allocator_, rhs.allocator_);
    std::swap(buffer_, rhs.buffer_);
    std::swap(allocation_, rhs.allocation_);
    std::swap(mapped_data_, rhs.mapped_data_);
  }
  return *this;
}

const vk::Buffer& Buffer::operator*() const noexcept { return buffer_; }

void Buffer::Clear() noexcept {
  if (allocator_ && buffer_ && allocation_) {
    vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer_), allocation_);
  }
  allocator_ = nullptr;
  buffer_ = nullptr;
  allocation_ = nullptr;
  mapped_data_ = nullptr;
}

void* Buffer::Map() {
  if (allocator_ && allocation_ && !mapped_data_) {
    vmaMapMemory(allocator_, allocation_, &mapped_data_);
  }

  return mapped_data_;
}

void Buffer::Unmap() {
  if (allocator_ && mapped_data_) {
    vmaUnmapMemory(allocator_, allocation_);
    mapped_data_ = nullptr;
  }
}

}  // namespace luka::gpu
