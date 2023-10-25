/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "function/rendering/dynamic_buffer.h"

#include "core/log.h"
#include "core/util.h"
#include "function/rendering/common.h"

namespace luka {

Ring::Ring(uint32_t total_size) : total_size_{total_size} {}
uint32_t Ring::GetTotalSize() const { return total_size_; }
uint32_t Ring::GetHead() const { return head_; }
uint32_t Ring::getAllocatedSize() const { return allocated_size_; }
uint32_t Ring::GetTail() const {
  return (head_ + allocated_size_) % total_size_;
}

DynamicBuffer::DynamicBuffer(const vk::raii::PhysicalDevice& physical_device,
                             const vk::raii::Device& device,
                             uint32_t mem_total_size,
                             uint32_t back_buffer_count)
    : ring_{AlignUp(mem_total_size, 256u)},
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

  data_ = reinterpret_cast<char*>(device_memory_.mapMemory(0, mem_requirements.size));
}

DynamicBuffer::~DynamicBuffer() { ring_.Free(ring_.getAllocatedSize()); }

void DynamicBuffer::BeginFrame(uint32_t back_buffer_index) {
  uint32_t mem_to_free{allocated_mem_per_back_buffer_[back_buffer_index]};
  ring_.Free(mem_to_free);
}

void DynamicBuffer::EndFrame(uint32_t back_buffer_index) {
  allocated_mem_per_back_buffer_[back_buffer_index] = mem_allocated_in_frame_;
  mem_allocated_in_frame_ = 0;
}

vk::DescriptorBufferInfo DynamicBuffer::AllocUniformBuffer(uint32_t size,
                                                           void* data) {
  void* buffer;
  vk::DescriptorBufferInfo out;
  AllocBuffer(size, &buffer, &out);
  memcpy(buffer, data, size);

  return out;
}

vk::DescriptorBufferInfo DynamicBuffer::AllocVertexBuffer(
    uint32_t vertices_count, uint32_t stride_in_byte, void** data) {
  vk::DescriptorBufferInfo out;
  AllocBuffer(vertices_count * stride_in_byte, data, &out);
  return out;
}

vk::DescriptorBufferInfo DynamicBuffer::AllocIndexBuffer(
    uint32_t indices_count, uint32_t stride_in_byte, void** data) {
  vk::DescriptorBufferInfo out;
  AllocBuffer(indices_count * stride_in_byte, data, &out);
  return out;
}

void DynamicBuffer::AllocBuffer(uint32_t size, void** data,
                                vk::DescriptorBufferInfo* out) {
  uint32_t align_up_size = AlignUp(size, 256u);

  uint32_t mem_offset;
  uint32_t padding{ring_.PaddingToAvoidCrossOver(align_up_size)};
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
