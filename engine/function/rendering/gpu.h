/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw

  Instance header file.
*/

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <map>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace luka {

class Window;

class Gpu {
 public:
  Gpu(std::shared_ptr<Window> window);
  ~Gpu();

 private:
  struct QueueFamliy {
    std::optional<uint32_t> graphics_index;
    std::optional<uint32_t> compute_index;
    std::optional<uint32_t> present_index;

    bool IsComplete() const {
      return graphics_index.has_value() && compute_index.has_value() &&
             present_index.has_value();
    }
  };

  void MakeBase();
  void MakeDevice();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  std::shared_ptr<Window> window_;

  vk::raii::Context context_;
  vk::raii::Instance instance_{nullptr};
#ifndef NDEBUG
  vk::raii::DebugUtilsMessengerEXT debug_utils_messenger_{nullptr};
#endif
  vk::raii::SurfaceKHR surface_{nullptr};
  vk::raii::PhysicalDevice physical_device_{nullptr};

  vk::SampleCountFlagBits sample_count_{vk::SampleCountFlagBits::e1};
  float max_anisotropy_{0.0f};
  QueueFamliy queue_family_;
  std::map<const char*, bool> device_extensions_{
      {VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, true},
      {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true},
      {VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, true}};
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue compute_queue_{nullptr};
  vk::raii::Queue present_queue_{nullptr};
};

}  // namespace luka