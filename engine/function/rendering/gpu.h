// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/dynamic_buffer.h"
#include "function/rendering/static_buffer.h"
#include "function/rendering/upload_buffer.h"

namespace luka {

class Window;

class Gpu {
 public:
  friend class Rendering;

  Gpu();
  ~Gpu();

  void Resize();
  void BeginFrame();
  void EndFrame();

  const vk::raii::CommandBuffer& GetCommandBuffer();

 private:
  void CreateInstance();
  void CreateSurface();
  void CreatePhysicalDevice();
  void CreateDevice();
  void CreateQueryPool();
  void CreateSwapchain();
  void CreateCommandObjects();
  void CreateSyncObjects();
  void CreateDescriptorPool();
  void CreateBuffers();
  void CreateRenderPass();
  void CreateFramebuffers();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  // Parameters.
  const u32 kBackBufferCount{3};
  u32 back_buffer_index{0};

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
  f32 max_anisotropy_{0.0f};
  std::optional<u32> graphics_queue_index_;
  std::optional<u32> compute_queue_index_;
  std::optional<u32> present_queue_index_;
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue compute_queue_{nullptr};
  vk::raii::Queue present_queue_{nullptr};

  // Query pool
  vk::raii::QueryPool query_pool_{nullptr};

  // Swapchain.
  u32 image_count_;
  vk::Format format_;
  vk::ColorSpaceKHR color_space_;
  vk::Extent2D extent_;
  vk::PresentModeKHR present_mode_;
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<vk::Image> images_;
  std::vector<vk::raii::ImageView> image_views_;

  // Command objects.
  const u32 kMaxUsedCommandBufferCountperFrame{8};
  std::vector<u32> used_command_buffer_counts_;
  std::vector<vk::raii::CommandPool> command_pools_;
  std::vector<vk::raii::CommandBuffers> command_buffers_;

  // Sync objects.
  std::vector<vk::raii::Fence> command_executed_fences_;
  std::vector<vk::raii::Semaphore> image_available_semaphores_;
  std::vector<vk::raii::Semaphore> render_finished_semaphores_;

  // Descriptor pool.
  vk::raii::DescriptorPool descriptor_pool_{nullptr};

  // Buffers.
  DynamicBuffer dynamic_buffer_;
  StaticBuffer device_static_buffer_;
  StaticBuffer host_static_buffer_;
  UploadBuffer upload_buffer_;

  // Render pass.
  vk::raii::RenderPass render_pass_{nullptr};

  // Framebuffers.
  std::vector<vk::raii::Framebuffer> framebuffers_;
};

}  // namespace luka