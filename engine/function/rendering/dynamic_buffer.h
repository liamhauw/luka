// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Ring {
 public:
  Ring(u32 total_size = 0);
  u32 GetTotalSize() const;
  u32 GetHead() const;
  u32 getAllocatedSize() const;
  u32 GetTail() const;

  u32 PaddingToAvoidCrossOver(u32 size) {
    u32 tail{GetTail()};
    if ((tail + size) > total_size_) {
      return total_size_ - tail;
    } else {
      return 0;
    }
  }

  bool Alloc(u32 size, u32* out) {
    if (allocated_size_ + size <= total_size_) {
      if (out) {
        *out = GetTail();
      }
      allocated_size_ += size;
      return true;
    }

    return false;
  }

  bool Free(u32 size) {
    if (allocated_size_ >= size) {
      head_ = (head_ + size) % total_size_;
      allocated_size_ -= size;
      return true;
    }
    return false;
  }

 private:
  u32 total_size_;
  u32 head_{0};
  u32 allocated_size_{0};
};

class DynamicBuffer {
 public:
  DynamicBuffer() = default;
  DynamicBuffer(const vk::raii::PhysicalDevice& physical_device,
                const vk::raii::Device& device, u32 mem_total_size,
                u32 back_buffer_count);
  DynamicBuffer& operator=(DynamicBuffer&&) = default;
  ~DynamicBuffer();

  void BeginFrame(u32 back_buffer_index);
  void EndFrame(u32 back_buffer_index);

  vk::DescriptorBufferInfo AllocUniformBuffer(u32 size, void* data);
  vk::DescriptorBufferInfo AllocVertexBuffer(u32 vertices_count,
                                             u32 stride_in_byte, void** data);
  vk::DescriptorBufferInfo AllocIndexBuffer(u32 indices_count,
                                            u32 stride_in_byte, void** data);

 private:
  void AllocBuffer(u32 size, void** data, vk::DescriptorBufferInfo* out);

  Ring ring_;
  u32 mem_allocated_in_frame_{0};
  std::vector<u32> allocated_mem_per_back_buffer_;
  vk::raii::Buffer buffer_{nullptr};
  vk::raii::DeviceMemory device_memory_{nullptr};
  u8* data_{nullptr};
};

}  // namespace luka
