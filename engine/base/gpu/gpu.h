// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
// clang-format on

#include <backends/imgui_impl_vulkan.h>
#include <tiny_gltf.h>

#include "base/gpu/buffer.h"
#include "base/gpu/image.h"
#include "base/window/window.h"
#include "core/util.h"

namespace luka {

class Gpu {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Gpu)

  explicit Gpu(std::shared_ptr<Window> window);

  ~Gpu();

  void Tick();

  vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities() const;
  std::vector<vk::SurfaceFormatKHR> GetSurfaceFormats() const;
  std::vector<vk::PresentModeKHR> GetSurfacePresentModes() const;
  vk::PhysicalDeviceProperties GetPhysicalDeviceProperties() const;

  u32 GetGraphicsQueueIndex() const;
  u32 GetComputeQueueIndex() const;
  u32 GetTransferQueueIndex() const;
  u32 GetPresentQueueIndex() const;

  const vk::raii::Queue& GetGraphicsQueue() const;
  const vk::raii::Queue& GetComputeQueue() const;
  const vk::raii::Queue& GetTransferQueue() const;
  const vk::raii::Queue& GetPresentQueue() const;

  ImGui_ImplVulkan_InitInfo GetImguiVulkanInitInfo() const;

  const vk::raii::Sampler& GetSampler() const;

  gpu::Buffer CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                           const void* data, bool map = false,
                           const std::string& name = {}, i32 index = -1);
  gpu::Buffer CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                           const gpu::Buffer& staging_buffer,
                           const vk::raii::CommandBuffer& command_buffer,
                           const std::string& name = {}, i32 index = -1);
  gpu::Image CreateImage(const vk::ImageCreateInfo& image_ci,
                         const std::string& name = {}, i32 index = -1);
  gpu::Image CreateImage(const vk::ImageCreateInfo& image_ci,
                         const vk::ImageLayout& new_layout,
                         const gpu::Buffer& staging_buffer,
                         const vk::raii::CommandBuffer& command_buffer,
                         const tinygltf::Image& tinygltf_image,
                         const std::string& name = {}, i32 index = -1);
  vk::raii::ImageView CreateImageView(
      const vk::ImageViewCreateInfo& image_view_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::Sampler CreateSampler(const vk::SamplerCreateInfo& sampler_ci,
                                  const std::string& name = {}, i32 index = -1);
  vk::raii::SwapchainKHR CreateSwapchain(
      vk::SwapchainCreateInfoKHR swapchain_ci, const std::string& name = {},
      i32 index = -1);
  vk::raii::Semaphore CreateSemaphoreLuka(
      const vk::SemaphoreCreateInfo& semaphore_ci, const std::string& name = {},
      i32 index = -1);
  vk::raii::Fence CreateFence(const vk::FenceCreateInfo& fence_ci,
                              const std::string& name = {}, i32 index = -1);
  vk::raii::CommandPool CreateCommandPool(
      const vk::CommandPoolCreateInfo& command_pool_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::RenderPass CreateRenderPass(
      const vk::RenderPassCreateInfo& render_pass_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::Framebuffer CreateFramebuffer(
      const vk::FramebufferCreateInfo& framebuffer_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::DescriptorPool CreateDescriptorPool(
      const vk::DescriptorPoolCreateInfo& descriptor_pool_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::DescriptorSetLayout CreateDescriptorSetLayout(
      const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::PipelineLayout CreatePipelineLayout(
      const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::ShaderModule CreateShaderModule(
      const vk::ShaderModuleCreateInfo& shader_module_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::PipelineCache CreatePipelineCache(
      const vk::PipelineCacheCreateInfo& pipeline_cache_ci,
      const std::string& name = {}, i32 index = -1);
  vk::raii::Pipeline CreatePipeline(
      const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci,
      const vk::raii::PipelineCache& pipeline_cache = nullptr,
      const std::string& name = {}, i32 index = -1);

  vk::raii::CommandBuffers AllocateCommandBuffers(
      const vk::CommandBufferAllocateInfo& command_buffer_ai,
      const std::string& name = {}, i32 index = -1);
  vk::raii::DescriptorSets AllocateNormalDescriptorSets(
      vk::DescriptorSetAllocateInfo descriptor_set_allocate_info,
      const std::string& name = {}, i32 index = -1);
  vk::raii::DescriptorSet AllocateBindlessDescriptorSet(
      vk::DescriptorSetAllocateInfo descriptor_set_allocate_info,
      const std::string& name = {}, i32 index = -1);

  void UpdateDescriptorSets(const std::vector<vk::WriteDescriptorSet>& writes);
  vk::Result WaitForFence(const vk::raii::Fence& fence);
  void ResetFence(const vk::raii::Fence& fence);
  void TransferQueueSubmit(const vk::SubmitInfo& submit_info);
  void WaitIdle();

  static void BeginLabel(const vk::raii::CommandBuffer& command_buffer,
                         const std::string& label,
                         const std::array<f32, 4>& color = {1.0F, 1.0F, 1.0F,
                                                            1.0F});
  static void EndLabel(const vk::raii::CommandBuffer& command_buffer);

 private:
  void CreateInstance();
  void CreateSurface();
  void CreatePhysicalDevice();
  void CreateDevice();
  void CreateVmaAllocator();
  void CreateDescriptorPool();
  void CreateDefaultResource();

  void DestroyAllocator();

  void SetObjectName(vk::ObjectType object_type, u64 handle,
                     const std::string& name, const std::string& prefix = {},
                     const std::string& suffix = {});

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
  f32 max_anisotropy_{0.0F};
  std::optional<u32> graphics_queue_index_;
  std::optional<u32> compute_queue_index_;
  std::optional<u32> transfer_queue_index_;
  std::optional<u32> present_queue_index_;
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue compute_queue_{nullptr};
  vk::raii::Queue transfer_queue_{nullptr};
  vk::raii::Queue present_queue_{nullptr};

  VmaAllocator allocator_{};

  vk::raii::DescriptorPool bindless_descriptor_pool_{nullptr};
  vk::raii::DescriptorPool normal_descriptor_pool_{nullptr};

  std::vector<std::unordered_map<std::string, vk::ImageView>>
      shared_image_views_;

  vk::raii::Sampler sampler_{nullptr};
};

}  // namespace luka