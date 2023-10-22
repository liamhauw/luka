/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "core/util.h"

namespace luka {

class Ring {
 public:
  Ring(uint32_t total_size = 0);

 private:
  uint32_t head_{0};
  uint32_t allocated_size_{0};

  uint32_t total_size_;
};

class RingWithTab {
 public:
  RingWithTab(uint32_t mem_total_size = 0, uint32_t back_buffer_count = 0);

 private:
  uint32_t back_buffer_index_{0};
  uint32_t mem_allocated_in_frame_{0};
  std::vector<uint32_t> allocated_mem_per_back_buffer_{
      std::vector<uint32_t>(4, 0)};

  Ring mem_;
  uint32_t back_buffer_count_;
};

class DynamicBufferRing {
 public:
  DynamicBufferRing() = default;
  DynamicBufferRing(const vk::raii::PhysicalDevice& physical_device,
                    const vk::raii::Device& device, uint32_t mem_total_size,
                    uint32_t back_buffer_count);

 private:
  RingWithTab mem_;
  uint32_t mem_total_size_;
  vk::raii::Buffer buffer_{nullptr};
  vk::raii::DeviceMemory device_memory_{nullptr};
  void* data_{nullptr};
};

}  // namespace luka
