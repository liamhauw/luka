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

#include "resource/asset/asset.h"

namespace luka {

class Asset;
class Window;

class Gpu {
 public:
  Gpu(std::shared_ptr<Asset> asset, std::shared_ptr<Window> window);
  ~Gpu();

  void Resize();
  void BeginFrame();
  void EndFrame();

  void MakePipeline(const std::vector<char>& vertex_shader_buffer,
                    const std::vector<char>& fragment_shader_buffer);

 private:
  void MakeInstance();
  void MakeSurface();
  void MakePhysicalDevice();
  void MakeDevice();
  void MakeCommandObjects();
  void MakeSyncObjects();
  void MakeSwapchain();
  void MakeColorImage();
  void MakeDepthImage();
  void MakeRenderPass();
  void MakeFramebuffer();

  void MakeVertexBuffer();
  void MakeIndexBuffer();
  void MakeUniformBuffer();
  void MakeTextureImage();
  void MakeTextureSampler();

  void MakeDescriptorSetLayout();
  void MakeDescriptorPool();
  void MakeDescriptorSet();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);

  vk::raii::DeviceMemory AllocateDeviceMemory(
      const vk::MemoryRequirements& memory_requirements,
      vk::MemoryPropertyFlags memory_properties_flags);

  template <typename T>
  void CopyToDevice(const vk::raii::DeviceMemory& device_memory, const T* data,
                    vk::DeviceSize buffer_size) {
    uint8_t* device_data{
        static_cast<uint8_t*>(device_memory.mapMemory(0, buffer_size))};
    memcpy(device_data, data, buffer_size);
    device_memory.unmapMemory();
  }

  void CopyBuffer(const vk::raii::Buffer& src_buffer,
                  const vk::raii::Buffer& dst_buffer, vk::DeviceSize size);

  vk::raii::CommandBuffer BeginSingleTimeCommand();

  void EndSingleTimeCommand(const vk::raii::CommandBuffer& command_buffer);

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

  struct BufferData {
    vk::raii::Buffer buffer{nullptr};
    vk::raii::DeviceMemory device_memory{nullptr};
  };

  std::shared_ptr<Asset> asset_;
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

  ImageData color_image_data_;

  ImageData depth_image_data_;

  vk::raii::RenderPass render_pass_{nullptr};

  std::vector<vk::raii::Framebuffer> framebuffers_;

  BufferData vertex_buffer_data_;
  BufferData index_buffer_data_;
  std::vector<BufferData> uniform_buffer_datas_;
  std::vector<void*> uniform_buffer_mapped_dates_;

  uint32_t mip_level_count_{};
  ImageData texture_image_data_;

  vk::raii::Sampler sampler_{nullptr};

  vk::raii::DescriptorSetLayout descriptor_set_layout_{nullptr};
  vk::raii::DescriptorPool descriptor_pool_{nullptr};
  std::vector<vk::raii::DescriptorSet> descriptor_sets_;

  vk::raii::PipelineLayout pipeline_layout_{nullptr};
  vk::raii::PipelineCache pipeline_cache_{nullptr};
  vk::raii::Pipeline pipeline_{nullptr};
};

}  // namespace luka
