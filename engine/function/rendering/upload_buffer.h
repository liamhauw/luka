// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class UploadBuffer {
 public:
  UploadBuffer() = default;
  UploadBuffer(const vk::raii::PhysicalDevice& physical_device,
               const vk::raii::Device& device, u32 graphics_queue_index,
               u32 mem_total_siz);

 private:
  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::CommandBuffer command_buffer_{nullptr};
  vk::raii::Fence fence_{nullptr};
  vk::raii::Buffer buffer_{nullptr};
  vk::raii::DeviceMemory device_memory_{nullptr};
  uint8_t* data_begin_{nullptr};
  uint8_t* data_cur_{nullptr};
  uint8_t* data_end_{nullptr};
};

}  // namespace luka
