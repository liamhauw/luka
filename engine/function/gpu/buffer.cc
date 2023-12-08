// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/buffer.h"

#include "core/log.h"

namespace luka {

Buffer::Buffer(const VmaAllocator& allocator,
               const vk::BufferCreateInfo& buffer_ci, bool staging)
    : allocator_{allocator} {
  VkBufferCreateInfo vk_buffer_ci{static_cast<VkBufferCreateInfo>(buffer_ci)};
  VmaAllocationCreateInfo allocation_ci{.usage = VMA_MEMORY_USAGE_AUTO};
  if (staging) {
    allocation_ci.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
  }
  VkBuffer buffer;
  vmaCreateBuffer(allocator_, &vk_buffer_ci, &allocation_ci, &buffer,
                  &allocation_, nullptr);
  buffer_ = buffer;
}

Buffer::~Buffer() { Clear(); }

Buffer::Buffer(Buffer&& rhs) noexcept
    : allocator_{rhs.allocator_},
      buffer_{std::exchange(rhs.buffer_, {})},
      allocation_{std::exchange(rhs.allocation_, {})} {}

Buffer& Buffer::operator=(Buffer&& rhs) noexcept {
  if (this != &rhs) {
    allocator_ = rhs.allocator_;
    std::swap(buffer_, rhs.buffer_);
    std::swap(allocation_, rhs.allocation_);
  }
  return *this;
}

const vk::Buffer& Buffer::operator*() const noexcept { return buffer_; }

void Buffer::Clear() noexcept {
  if (buffer_) {
    vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer_), allocation_);
  }
  allocator_ = nullptr;
  buffer_ = nullptr;
  allocation_ = nullptr;
}
const VmaAllocator& Buffer::GetAllocator() const { return allocator_; }

const VmaAllocation& Buffer::GetAllocation() const { return allocation_; }

}  // namespace luka
