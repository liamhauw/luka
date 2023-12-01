// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

#include "function/gpu/buffer.h"
#include "function/gpu/image.h"
#include "imgui_impl_vulkan.h"

namespace luka {

class Gpu {
 public:
  Gpu();
  ~Gpu();

  void Tick();

  ImGui_ImplVulkan_InitInfo GetVulkanInitInfoForImgui();
  VkRenderPass GetRenderPassForImGui();
  const vk::Extent2D& GetExtent2D() const;

  Buffer CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                      bool staging = false, u64 size = 0,
                      const void* data = nullptr, const std::string& name = {});
  Image CreateImage(
      const vk::ImageCreateInfo& image_ci,
      const vk::ImageLayout new_layout = vk::ImageLayout::eUndefined,
      u64 size = 0, const void* data = nullptr, const std::string& name = {});

  vk::raii::ImageView CreateImageView(
      const vk::ImageViewCreateInfo& image_view_ci,
      const std::string& name = {});

  vk::raii::Sampler CreateSampler(const vk::SamplerCreateInfo sampler_ci,
                                  const std::string& name = {});

  vk::raii::PipelineLayout CreatePipelineLayout(
      const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
      const std::string& name = {});

  vk::raii::Pipeline CreatePipeline(
      const std::vector<u8>& vertex_shader_buffer,
      const std::vector<u8>& fragment_shader_buffer, uint32_t vertex_stride,
      const std::vector<std::pair<vk::Format, uint32_t>>&
          vertex_input_attribute_format_offset,
      const vk::raii::PipelineLayout& pipeline_layout,
      const vk::PipelineRenderingCreateInfo& pipeline_rendering_ci,
      const std::string& name = {});

  
  const vk::raii::CommandBuffer& GetCommandBuffer();
  vk::raii::CommandBuffer BeginTempCommandBuffer();
  void EndTempCommandBuffer(const vk::raii::CommandBuffer& command_buffer);
  const vk::raii::CommandBuffer& BeginFrame();
  void EndFrame(const vk::raii::CommandBuffer& cur_command_buffer);
  void BeginRenderPass(const vk::raii::CommandBuffer& cur_command_buffer);
  void EndRenderPass(const vk::raii::CommandBuffer& cur_command_buffer);
  void WaitIdle();

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
  void CreateAllocator();

  void Resize();
  
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

  // Allocator.
  VmaAllocator allocator_;
};

}  // namespace luka