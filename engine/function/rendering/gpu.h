/*
  SPDX license identifier: MIT

  Copyright (c) 2023-present Liam Hauw.

  GPU header file.
*/

#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace luka {

class Window;

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
  uint32_t count;
  vk::Format format;
  vk::ColorSpaceKHR color_space;
  vk::Extent2D extent;
  vk::raii::SwapchainKHR swapchain{nullptr};
  std::vector<vk::Image> images;
  std::vector<vk::raii::ImageView> image_views;
};

struct ImageData {
  vk::Format format;
  vk::raii::Image image{nullptr};
  vk::raii::DeviceMemory device_memory{nullptr};
  vk::raii::ImageView image_view{nullptr};
};

class Gpu {
 public:
  Gpu(std::shared_ptr<Window> window);
  ~Gpu();

  void MakeGraphicsPipeline(const std::vector<char>& vectex_shader_buffer,
                            const std::vector<char>& fragment_shader_buffer);

  void Resize();
  void BeginFrame();
  void EndFrame();

 private:
  void MakeInstance();
  void MakeSurface();
  void MakePhysicalDevice();
  void MakeDevice();
  void MakeCommandObjects();
  void MakeSyncObjects();
  void MakeSwapchain();
  void MakeDepthImage();
  void MakeRenderPass();
  void MakeFramebuffer();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  static vk::raii::DeviceMemory AllocateDeviceMemory(
      const vk::raii::PhysicalDevice& physical_device,
      const vk::raii::Device& device,
      const vk::MemoryRequirements& memory_requirements,
      vk::MemoryPropertyFlags memory_properties_flags);

  std::shared_ptr<Window> window_;

  const uint32_t kFramesInFlight{2};
  uint32_t current_frame_{0};

  vk::raii::Context context_;
  std::vector<const char*> required_instance_layers_;
  std::vector<const char*> required_instance_extensions_;
  vk::raii::Instance instance_{nullptr};
#ifndef NDEBUG
  vk::raii::DebugUtilsMessengerEXT debug_utils_messenger_{nullptr};
#endif

  vk::raii::SurfaceKHR surface_{nullptr};

  vk::raii::PhysicalDevice physical_device_{nullptr};
  QueueFamliy queue_family_;
  std::vector<const char*> required_device_extensions_{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};
  vk::SampleCountFlagBits sample_count_{vk::SampleCountFlagBits::e1};
  float max_anisotropy_{0.0f};

  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue compute_queue_{nullptr};
  vk::raii::Queue present_queue_{nullptr};

  vk::raii::CommandPool command_pool_{nullptr};
  std::vector<vk::raii::CommandBuffer> command_buffers_;

  std::vector<vk::raii::Fence> fences_;
  std::vector<vk::raii::Semaphore> image_available_semaphores_;
  std::vector<vk::raii::Semaphore> render_finished_semaphores_;

  SwapchainData swapchain_data_;
  uint32_t image_index_;

  ImageData depth_image_data_;

  vk::raii::RenderPass render_pass_{nullptr};

  std::vector<vk::raii::Framebuffer> framebuffers_;

  vk::raii::PipelineLayout pipeline_layout_{nullptr};
  vk::raii::PipelineCache pipeline_cache_{nullptr};
  vk::raii::Pipeline pipeline_{nullptr};
};

}  // namespace luka
