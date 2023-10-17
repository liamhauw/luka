/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Instance header file.
*/

#pragma once

#include <function/window/window.h>

#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace luka {

class Instance {
 public:
  Instance(const std::vector<const char*>& required_instance_layers = {},
           const std::vector<const char*>& required_instance_extensions = {});

 private:
  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  vk::raii::Context context_;
  std::vector<const char*> required_instance_layers_;
  std::vector<const char*> required_instance_extensions_;
  vk::raii::Instance instance_{nullptr};
#ifndef NDEBUG
  vk::raii::DebugUtilsMessengerEXT debug_utils_messenger_{nullptr};
#endif
};
}  // namespace luka