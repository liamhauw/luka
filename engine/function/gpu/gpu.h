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
#include "function/window/window.h"
#include "imgui_impl_vulkan.h"
#include "resource/asset/image.h"

namespace luka {

class Gpu {
 public:
  Gpu(std::shared_ptr<Window> window);
  ~Gpu();

  void Tick();

  gpu::Buffer CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                           const void* data, const std::string& name = {});

  gpu::Buffer CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                           const gpu::Buffer& staging_buffer,
                           const vk::raii::CommandBuffer& command_buffer,
                           const std::string& name = {});

  gpu::Image CreateImage(const vk::ImageCreateInfo& image_ci,
                         const std::string& name = {});

  gpu::Image CreateImage(const vk::ImageCreateInfo& image_ci,
                         const vk::ImageLayout new_layout,
                         const gpu::Buffer& staging_buffer,
                         const vk::raii::CommandBuffer& command_buffer,
                         const ast::Image& asset_image,
                         const std::string& name = {});

  vk::raii::ImageView CreateImageView(
      const vk::ImageViewCreateInfo& image_view_ci,
      const std::string& name = {});

  vk::raii::Sampler CreateSampler(const vk::SamplerCreateInfo sampler_ci,
                                  const std::string& name = {});

  vk::raii::SwapchainKHR CreateSwapchain(
      vk::SwapchainCreateInfoKHR swapchain_ci, const std::string& name = {});

  vk::raii::CommandPool CreateCommandPool(
      const vk::CommandPoolCreateInfo& command_pool_ci,
      const std::string& name = {});

  vk::raii::CommandBuffers AllocateCommandBuffers(
      const vk::CommandBufferAllocateInfo& command_buffer_ai,
      const std::string& name = {});

  vk::raii::Fence CreateFence(const vk::FenceCreateInfo& fence_ci,
                              const std::string& name = {});

  vk::raii::Semaphore CreateSemaphore0(
      const vk::SemaphoreCreateInfo& semaphore_ci,
      const std::string& name = {});

  u32 GetGraphicsQueueIndex() const;

  u32 GetPresentQueueIndex() const;

  const vk::SurfaceCapabilitiesKHR& GetSurfaceCapabilities() const;

  const std::vector<vk::SurfaceFormatKHR>& GetSurfaceFormats() const;

  const std::vector<vk::PresentModeKHR>& GetSurfacePresentModes() const;

  vk::raii::CommandBuffer BeginTempCommandBuffer();

  void EndTempCommandBuffer(const vk::raii::CommandBuffer& command_buffer);

  void WaitIdle();

  //   vk::raii::DescriptorSetLayout CreateDescriptorSetLayout(
  //       const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
  //       const std::string& name = {});

  //   vk::raii::PipelineLayout CreatePipelineLayout(
  //       const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
  //       const std::string& name = {});

  //   vk::raii::Pipeline CreatePipeline(
  //       const std::vector<u8>& vertex_shader_buffer,
  //       const std::vector<u8>& fragment_shader_buffer,
  //       const std::vector<std::pair<u32, vk::Format>>&
  //       vertex_input_stride_format, const
  //       std::vector<vk::raii::DescriptorSetLayout>& descriptor_set_layout,
  //       const vk::PipelineRenderingCreateInfo& pipeline_rendering_ci,
  //       const std::string& name = {});

  // const u32 GetBackBufferCount() const;

  //   vk::raii::DescriptorSet AllocateDescriptorSet(
  //       vk::DescriptorSetAllocateInfo descriptor_set_allocate_info,
  //       const std::string& name = {});

  //   void UpdateDescriptorSets(const std::vector<vk::WriteDescriptorSet>&
  //   writes);

  //   const vk::raii::CommandBuffer& BeginFrame();

  //   void EndFrame(const vk::raii::CommandBuffer& cur_command_buffer);

  //   void BeginLabel(const vk::raii::CommandBuffer& command_buffer,
  //                   const std::string& label,
  //                   const std::array<f32, 4>& color = {0.0F, 0.0F,
  //                   0.6F, 1.0F});

  //   void EndLabel(const vk::raii::CommandBuffer& command_buffer);

  //   void BeginRenderPass(const vk::raii::CommandBuffer& cur_command_buffer);

  //   void EndRenderPass(const vk::raii::CommandBuffer& cur_command_buffer);

  //   const vk::Extent2D& GetExtent2D() const;

  //   const vk::raii::DescriptorSet& GetBindlessDescriptorSet() const;

  //   const vk::raii::PipelineLayout& GetPipelineLayout() const;

  //   std::pair<ImGui_ImplVulkan_InitInfo, VkRenderPass>
  //   GetVulkanInfoForImgui()
  //       const;

 private:
  void CreateInstance();
  void CreateSurface();
  void CreatePhysicalDevice();
  void CreateDevice();
  void CreateAllocator();
  void CreateCommandObjects();

  void SetObjectName(vk::ObjectType object_type, u64 handle,
                     const std::string& name, const std::string& suffix = {});

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  //   void CreateSyncObjects();
  //   void CreateSwapchain();
  //   void CreateRenderPass();
  //   void CreateFramebuffers();
  //   void CreatePipelineCache();
  //   void CreateDescriptorObjects();
  //   void Resize();
  //   const vk::raii::CommandBuffer& GetCommandBuffer();

  std::shared_ptr<Window> window_;

  // Parameters.
  const u32 kBackBufferCount{1};
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
  vk::SurfaceCapabilitiesKHR surface_capabilities_;
  std::vector<vk::SurfaceFormatKHR> surface_formats_;
  std::vector<vk::PresentModeKHR> present_modes_;

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

  // Allocator.
  VmaAllocator allocator_;

  // Command objects.
  const u32 kMaxUsedCommandBufferCountperFrame{8};
  std::vector<u32> used_command_buffer_counts_;
  std::vector<vk::raii::CommandPool> command_pools_;
  std::vector<vk::raii::CommandBuffers> command_buffers_;

  //   // Sync objects.
  //   std::vector<vk::raii::Fence> command_executed_fences_;
  //   std::vector<vk::raii::Semaphore> image_available_semaphores_;
  //   std::vector<vk::raii::Semaphore> render_finished_semaphores_;

  //   // Swapchain.
  //   u32 image_count_;
  //   vk::Format format_;
  //   vk::ColorSpaceKHR color_space_;
  //   vk::Extent2D extent_;
  //   vk::PresentModeKHR present_mode_;
  //   vk::raii::SwapchainKHR swapchain_{nullptr};
  //   std::vector<vk::Image> images_;
  //   std::vector<vk::raii::ImageView> image_views_;

  //   // Render pass.
  //   vk::raii::RenderPass render_pass_{nullptr};

  //   // Framebuffers.
  //   std::vector<vk::raii::Framebuffer> framebuffers_;

  //   // Pipeline cache.
  //   vk::raii::PipelineCache pipeline_cache_{nullptr};
  //   vk::raii::PipelineLayout pipeline_layout_{nullptr};

  //   // Descriptor objects.
  //   vk::raii::DescriptorPool descriptor_pool_{nullptr};
  //   const u32 kBindlessDescriptorCount{1024};
  //   const u32 kBindlessBinding{10};
  //   vk::raii::DescriptorPool bindless_descriptor_pool_{nullptr};
  //   vk::raii::DescriptorSetLayout bindless_descriptor_set_layout_{nullptr};
  //   vk::raii::DescriptorSet bindless_descriptor_set{nullptr};
};

}  // namespace luka