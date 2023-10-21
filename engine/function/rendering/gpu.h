/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw.
*/

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace luka {

class Window;

class Gpu {
 public:
  Gpu(std::shared_ptr<Window> window);
  ~Gpu();

  void Resize();

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

  struct SwapchainData {
    uint32_t image_count;
    vk::Format format;
    vk::ColorSpaceKHR color_space;
    vk::Extent2D extent;
    vk::PresentModeKHR present_mode;
    vk::raii::SwapchainKHR swapchain{nullptr};
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> image_views;
  };

  void CreateInstance();
  void CreateSurface();
  void CreatePhysicalDevice();
  void CreateDevice();
  void CreateSwapchain();
  void CreateSyncObjects();
  void CreateCommandObjects();
  void CreateDescriptorPool();
  void CreateRenderPass();
  void CreateFramebuffers();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  // Parameters.
  const uint32_t kFramesInFlight{3};

  // Window.
  std::shared_ptr<Window> window_;

  // Instance.
  vk::raii::Context context_;
  vk::raii::Instance instance_{nullptr};
#ifndef NDEBUG
  vk::raii::DebugUtilsMessengerEXT debug_utils_messenger_{nullptr};
#endif

  // Surface.
  vk::raii::SurfaceKHR surface_{nullptr};

  // Physical device.
  vk::raii::PhysicalDevice physical_device_{nullptr};

  // Device.
  vk::SampleCountFlagBits sample_count_{vk::SampleCountFlagBits::e1};
  float max_anisotropy_{0.0f};
  QueueFamliy queue_family_;
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue compute_queue_{nullptr};
  vk::raii::Queue present_queue_{nullptr};

  // Swapchain.
  SwapchainData swapchain_data_;

  // Sync objects
  std::vector<vk::raii::Fence> command_executed_fences_;
  std::vector<vk::raii::Semaphore> image_available_semaphores_;
  std::vector<vk::raii::Semaphore> render_finished_semaphores_;

  // Command objects.
  std::vector<vk::raii::CommandPool> command_pools_;
  std::vector<vk::raii::CommandBuffers> command_buffers_;

  // Descriptor pool.
  vk::raii::DescriptorPool descriptor_pool_{nullptr};

  // Render pass.
  vk::raii::RenderPass render_pass_{nullptr};

  // Framebuffers.
  std::vector<vk::raii::Framebuffer> framebuffers_;
};

}  // namespace luka