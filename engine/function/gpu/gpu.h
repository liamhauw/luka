// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

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

struct ResourceCache {
  std::unordered_map<u64, vk::raii::DescriptorSetLayout> descriptor_set_layouts;
  std::unordered_map<u64, vk::raii::PipelineLayout> pipeline_layouts;
  std::unordered_map<u64, vk::raii::ShaderModule> shader_modules;
  std::unordered_map<u64, vk::raii::Pipeline> pipelines;
};

class Gpu {
 public:
  Gpu(std::shared_ptr<Window> window);
  ~Gpu();

  void Tick();

  gpu::Buffer CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                           const void* data, bool map = false,
                           const std::string& name = {});

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

  vk::raii::Semaphore CreateSemaphoreLuka(
      const vk::SemaphoreCreateInfo& semaphore_ci,
      const std::string& name = {});

  vk::raii::Fence CreateFence(const vk::FenceCreateInfo& fence_ci,
                              const std::string& name = {});

  vk::raii::CommandPool CreateCommandPool(
      const vk::CommandPoolCreateInfo& command_pool_ci,
      const std::string& name = {});

  vk::raii::RenderPass CreateRenderPass(
      const vk::RenderPassCreateInfo& render_pass_ci,
      const std::string& name = {});

  vk::raii::Framebuffer CreateFramebuffer(
      const vk::FramebufferCreateInfo& framebuffer_ci,
      const std::string& name = {});

  const vk::raii::DescriptorSetLayout& RequestDescriptorSetLayout(
      const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
      const std::string& name = {});

  const vk::raii::PipelineLayout& RequestPipelineLayout(
      const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
      const std::string& name = {});

  const vk::raii::ShaderModule& RequestShaderModule(
      const vk::ShaderModuleCreateInfo& shader_module_ci,
      const std::string& name = {});

  const vk::raii::Pipeline& RequestPipeline(
      const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci,
      const std::string& name = {});

  vk::raii::CommandBuffers AllocateCommandBuffers(
      const vk::CommandBufferAllocateInfo& command_buffer_ai,
      const std::string& name = {});

  vk::raii::DescriptorSets AllocateDescriptorSets(
      vk::DescriptorSetAllocateInfo descriptor_set_allocate_info,
      const std::string& name = {});

  void UpdateDescriptorSets(const std::vector<vk::WriteDescriptorSet>& writes);

  vk::Result WaitForFence(const vk::raii::Fence& fence);

  void ResetFence(const vk::raii::Fence& fence);

  const vk::raii::CommandBuffer& BeginTempCommandBuffer();

  void EndTempCommandBuffer(const vk::raii::CommandBuffer& command_buffer);

  void WaitIdle();

  void BeginLabel(const vk::raii::CommandBuffer& command_buffer,
                  const std::string& label,
                  const std::array<f32, 4>& color = {0.8F, 0.7F, 1.0F, 1.0F});

  void EndLabel(const vk::raii::CommandBuffer& command_buffer);

  const vk::raii::Queue& GetGraphicsQueue() const;

  const vk::raii::Queue& GetPresentQueue() const;

  u32 GetGraphicsQueueIndex() const;

  u32 GetPresentQueueIndex() const;

  vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities() const;

  std::vector<vk::SurfaceFormatKHR> GetSurfaceFormats() const;

  std::vector<vk::PresentModeKHR> GetSurfacePresentModes() const;

 private:
  void CreateInstance();
  void CreateSurface();
  void CreatePhysicalDevice();
  void CreateDevice();
  void CreateAllocator();
  void CreateCommandObjects();
  void CreateDescriptorPool();

  void SetObjectName(vk::ObjectType object_type, u64 handle,
                     const std::string& name, const std::string& suffix = {});

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
  f32 max_anisotropy_{0.0f};
  std::optional<u32> graphics_queue_index_;
  std::optional<u32> compute_queue_index_;
  std::optional<u32> present_queue_index_;
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue compute_queue_{nullptr};
  vk::raii::Queue present_queue_{nullptr};

  VmaAllocator allocator_;

  const u32 kCommandBufferCount{1};
  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::CommandBuffers command_buffers_{nullptr};

  vk::raii::DescriptorPool descriptor_pool_{nullptr};

  ResourceCache resource_cache_;
};

}  // namespace luka