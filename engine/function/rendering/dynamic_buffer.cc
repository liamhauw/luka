// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/dynamic_buffer.h"

#include "core/log.h"
#include "core/util.h"
#include "function/rendering/common.h"

namespace luka {

Ring::Ring(u32 total_size) : total_size_{total_size} {}
u32 Ring::GetTotalSize() const { return total_size_; }
u32 Ring::GetHead() const { return head_; }
u32 Ring::getAllocatedSize() const { return allocated_size_; }
u32 Ring::GetTail() const { return (head_ + allocated_size_) % total_size_; }

DynamicBuffer::DynamicBuffer(const vk::raii::PhysicalDevice& physical_device,
                             const vk::raii::Device& device, u32 mem_total_size,
                             u32 back_buffer_count)
    : ring_{AlignUp(mem_total_size, 256U)},
      allocated_mem_per_back_buffer_(back_buffer_count, 0) {
  vk::BufferCreateInfo buffer_ci{{},
                                 ring_.GetTotalSize(),
                                 vk::BufferUsageFlagBits::eUniformBuffer |
                                     vk::BufferUsageFlagBits::eIndexBuffer |
                                     vk::BufferUsageFlagBits::eVertexBuffer};
  buffer_ = vk::raii::Buffer{device, buffer_ci};

  vk::MemoryRequirements mem_requirements{buffer_.getMemoryRequirements()};

  device_memory_ = vk::raii::DeviceMemory{
      AllocateDeviceMemory(physical_device, device, mem_requirements,
                           vk::MemoryPropertyFlagBits::eHostVisible |
                               vk::MemoryPropertyFlagBits::eHostCoherent)};

  buffer_.bindMemory(*(device_memory_), 0);

  data_ = reinterpret_cast<char*>(
      device_memory_.mapMemory(0, mem_requirements.size));
}

DynamicBuffer::~DynamicBuffer() { ring_.Free(ring_.getAllocatedSize()); }

void DynamicBuffer::BeginFrame(u32 back_buffer_index) {
  u32 mem_to_free{allocated_mem_per_back_buffer_[back_buffer_index]};
  ring_.Free(mem_to_free);
}

void DynamicBuffer::EndFrame(u32 back_buffer_index) {
  allocated_mem_per_back_buffer_[back_buffer_index] = mem_allocated_in_frame_;
  mem_allocated_in_frame_ = 0;
}

vk::DescriptorBufferInfo DynamicBuffer::AllocUniformBuffer(u32 size,
                                                           void* data) {
  void* buffer;
  vk::DescriptorBufferInfo out;
  AllocBuffer(size, &buffer, &out);
  memcpy(buffer, data, size);

  return out;
}

vk::DescriptorBufferInfo DynamicBuffer::AllocVertexBuffer(u32 vertices_count,
                                                          u32 stride_in_byte,
                                                          void** data) {
  vk::DescriptorBufferInfo out;
  AllocBuffer(vertices_count * stride_in_byte, data, &out);
  return out;
}

vk::DescriptorBufferInfo DynamicBuffer::AllocIndexBuffer(u32 indices_count,
                                                         u32 stride_in_byte,
                                                         void** data) {
  vk::DescriptorBufferInfo out;
  AllocBuffer(indices_count * stride_in_byte, data, &out);
  return out;
}

void DynamicBuffer::AllocBuffer(u32 size, void** data,
                                vk::DescriptorBufferInfo* out) {
  u32 align_up_size = AlignUp(size, 256U);

  u32 mem_offset;
  u32 padding{ring_.PaddingToAvoidCrossOver(align_up_size)};
  if (padding > 0) {
    if (!ring_.Alloc(padding, nullptr)) {
      THROW("Fail to alloc padding.");
    }
    mem_allocated_in_frame_ += padding;
  }

  if (!ring_.Alloc(align_up_size, &mem_offset)) {
    THROW("Fail to alloc align_up_size.");
  }

  mem_allocated_in_frame_ += align_up_size;

  *data = reinterpret_cast<void*>(data_ + mem_offset);

  out->buffer = *buffer_;
  out->offset = mem_offset;
  out->range = align_up_size;
}

}  // namespace luka
