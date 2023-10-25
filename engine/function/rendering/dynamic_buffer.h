/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace luka {

class Ring {
 public:
  Ring(uint32_t total_size = 0);
  uint32_t GetTotalSize() const;
  uint32_t GetHead() const;
  uint32_t getAllocatedSize() const;
  uint32_t GetTail() const;

  uint32_t PaddingToAvoidCrossOver(uint32_t size) {
    uint32_t tail{GetTail()};
    if ((tail + size) > total_size_) {
      return total_size_ - tail;
    } else {
      return 0;
    }
  }

  bool Alloc(uint32_t size, uint32_t* out) {
    if (allocated_size_ + size <= total_size_) {
      if (out) {
        *out = GetTail();
      }
      allocated_size_ += size;
      return true;
    }

    return false;
  }

  bool Free(uint32_t size) {
    if (allocated_size_ >= size) {
      head_ = (head_ + size) % total_size_;
      allocated_size_ -= size;
      return true;
    }
    return false;
  }

 private:
  uint32_t total_size_;
  uint32_t head_{0};
  uint32_t allocated_size_{0};
};

class DynamicBuffer {
 public:
  DynamicBuffer() = default;
  DynamicBuffer(const vk::raii::PhysicalDevice& physical_device,
                const vk::raii::Device& device, uint32_t mem_total_size,
                uint32_t back_buffer_count);
  DynamicBuffer& operator=(DynamicBuffer&&) = default;
  ~DynamicBuffer();

  void BeginFrame(uint32_t back_buffer_index);
  void EndFrame(uint32_t back_buffer_index);

  vk::DescriptorBufferInfo AllocUniformBuffer(uint32_t size, void* data);
  vk::DescriptorBufferInfo AllocVertexBuffer(uint32_t vertices_count,
                                             uint32_t stride_in_byte,
                                             void** data);
  vk::DescriptorBufferInfo AllocIndexBuffer(uint32_t indices_count,
                                            uint32_t stride_in_byte,
                                            void** data);

 private:
  void AllocBuffer(uint32_t size, void** data, vk::DescriptorBufferInfo* out);

  Ring ring_;
  uint32_t mem_allocated_in_frame_{0};
  std::vector<uint32_t> allocated_mem_per_back_buffer_;
  vk::raii::Buffer buffer_{nullptr};
  vk::raii::DeviceMemory device_memory_{nullptr};
  char* data_{nullptr};
};

}  // namespace luka
