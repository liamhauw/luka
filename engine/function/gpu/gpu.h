// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

#include "imgui_impl_vulkan.h"

namespace luka {

class Buffer {
 public:
  Buffer() = delete;
  Buffer(std::nullptr_t) {}
  Buffer(const vk::raii::Device& device, const VmaAllocator& allocator,
         const vk::BufferCreateInfo& buffer_ci, u64 size = 0,
         const void* data = nullptr, bool staging = false,
         const vk::raii::CommandBuffer& = nullptr);
  ~Buffer();
  Buffer(const Buffer&) = delete;
  Buffer(Buffer&& rhs) noexcept;
  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&& rhs) noexcept;

  const vk::Buffer& operator*() const noexcept;

 private:
  VmaAllocator allocator_;
  vk::Buffer buffer_{nullptr};
  VmaAllocation allocation_{nullptr};
};

class Image {
 public:
  Image() = delete;
  Image(std::nullptr_t) {}
  Image(const vk::raii::Device& device, const VmaAllocator& allocator,
        const vk::ImageCreateInfo& image_ci);
  ~Image();
  Image(const Image&) = delete;
  Image(Image&& rhs) noexcept;
  Image& operator=(const Image&) = delete;
  Image& operator=(Image&& rhs) noexcept;

  const vk::Image& operator*() const noexcept;
  const vk::DescriptorImageInfo& GetDescriptorImageInfo() const;

 private:
  VmaAllocator allocator_;
  vk::Image image_{nullptr};
  VmaAllocation allocation_{nullptr};
  vk::raii::ImageView image_view_{nullptr};
  vk::raii::Sampler sampler_{nullptr};
  vk::DescriptorImageInfo descriptor_image_info_;
};

class Gpu {
 public:
  Gpu();
  ~Gpu();

  void Tick();

  void WaitIdle();

  Image CreateImage(const vk::ImageCreateInfo& image_ci, u64 size = 0,
                    const void* data = nullptr);

  const vk::raii::CommandBuffer& BeginFrame();
  void EndFrame(const vk::raii::CommandBuffer& cur_command_buffer);
  const vk::raii::CommandBuffer& GetCommandBuffer();

  ImGui_ImplVulkan_InitInfo GetVulkanInitInfoForImgui();
  VkRenderPass GetRenderPassForImGui();

 private:
  void CreateInstance();
  void CreateSurface();
  void CreatePhysicalDevice();
  void CreateDevice();
  void CreateQueryPool();
  void CreateSwapchain();
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateCommandObjects();
  void CreateSyncObjects();
  void CreatePipelineCache();
  void CreateDescriptorPool();
  void CreateVmaAllocator();

  void Resize();
  vk::raii::CommandBuffer BeginTempCommandBuffer();
  void EndTempCommandBuffer(const vk::raii::CommandBuffer& command_buffer);

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  // Parameters.
  const u32 kBackBufferCount{3};
  u32 back_buffer_index{0};
  u32 image_index_{0};

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

  // Render pass.
  vk::raii::RenderPass render_pass_{nullptr};

  // Framebuffers.
  std::vector<vk::raii::Framebuffer> framebuffers_;

  // Command objects.
  const u32 kMaxUsedCommandBufferCountperFrame{8};
  std::vector<u32> used_command_buffer_counts_;
  std::vector<vk::raii::CommandPool> command_pools_;
  std::vector<vk::raii::CommandBuffers> command_buffers_;

  // Sync objects.
  std::vector<vk::raii::Fence> command_executed_fences_;
  std::vector<vk::raii::Semaphore> image_available_semaphores_;
  std::vector<vk::raii::Semaphore> render_finished_semaphores_;

  // Pipeline cache.
  vk::raii::PipelineCache pipeline_cache_{nullptr};

  // Descriptor pool.
  vk::raii::DescriptorPool descriptor_pool_{nullptr};

  // Vma allocator.
  VmaAllocator vma_allocator_;
};

}  // namespace luka