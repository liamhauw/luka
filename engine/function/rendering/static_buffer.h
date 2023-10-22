/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace luka {

class StaticBuffer {
 public:
  StaticBuffer() = default;
  StaticBuffer(const vk::raii::PhysicalDevice& physical_device,
               const vk::raii::Device& device, uint32_t mem_total_size,
               bool is_device);

 private:
  uint32_t mem_total_size_;
  bool is_device_;
  vk::raii::Buffer host_buffer_{nullptr};
  vk::raii::DeviceMemory host_memory_{nullptr};
  void* data_{nullptr};
  vk::raii::Buffer device_buffer_{nullptr};
  vk::raii::DeviceMemory device_device_memory_{nullptr};
};

}  // namespace luka