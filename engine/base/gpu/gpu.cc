// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/gpu/gpu.h"

#include "core/log.h"
#include "core/util.h"

namespace luka {

Gpu::Gpu(std::shared_ptr<Window> window) : window_{std::move(window)} {
  CreateInstance();
  CreateSurface();
  CreatePhysicalDevice();
  CreateDevice();
  CreateVmaAllocator();
  CreateDescriptorPool();
  CreateDefaultResource();
}

Gpu::~Gpu() { DestroyAllocator(); }

void Gpu::Tick() {}

vk::PhysicalDeviceProperties Gpu::GetPhysicalDeviceProperties() const {
  return physical_device_.getProperties();
}

vk::SurfaceCapabilitiesKHR Gpu::GetSurfaceCapabilities() const {
  return physical_device_.getSurfaceCapabilitiesKHR(*surface_);
}

std::vector<vk::SurfaceFormatKHR> Gpu::GetSurfaceFormats() const {
  return physical_device_.getSurfaceFormatsKHR(*surface_);
}

std::vector<vk::PresentModeKHR> Gpu::GetSurfacePresentModes() const {
  return physical_device_.getSurfacePresentModesKHR(*surface_);
}

bool Gpu::HasIndexTypeUint8() const { return has_index_type_uint8_; }

u32 Gpu::GetGraphicsQueueIndex() const { return graphics_queue_index_.value(); }

u32 Gpu::GetComputeQueueIndex() const { return compute_queue_index_.value(); }

u32 Gpu::GetTransferQueueIndex() const { return transfer_queue_index_.value(); }

u32 Gpu::GetPresentQueueIndex() const { return present_queue_index_.value(); }

ImGui_ImplVulkan_InitInfo Gpu::GetImguiVulkanInitInfo() const {
  ImGui_ImplVulkan_InitInfo init_info{
      .Instance = static_cast<VkInstance>(*instance_),
      .PhysicalDevice = static_cast<VkPhysicalDevice>(*physical_device_),
      .Device = static_cast<VkDevice>(*device_),
      .QueueFamily = graphics_queue_index_.value(),
      .Queue = static_cast<VkQueue>(*graphics_queue_),
      .DescriptorPool = static_cast<VkDescriptorPool>(*normal_descriptor_pool_),
      .RenderPass = nullptr,
      .MinImageCount = 3,
      .ImageCount = 3,
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
      .PipelineCache = nullptr,
      .Subpass = 0,
  };

  return init_info;
}

const vk::raii::Sampler& Gpu::GetSampler() const { return sampler_; }

gpu::Buffer Gpu::CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                              const void* data, bool map,
                              const std::string& name, i32 index) {
  gpu::Buffer buffer{allocator_, buffer_ci, true};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eBuffer,
                reinterpret_cast<uint64_t>(static_cast<VkBuffer>(*buffer)),
                name, "Buffer", index == -1 ? "" : std::to_string(index));
#endif

  void* mapped_data{buffer.Map()};
  memcpy(mapped_data, data, buffer_ci.size);
  if (!map) {
    buffer.Unmap();
  }

  return buffer;
}

gpu::Buffer Gpu::CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                              const gpu::Buffer& staging_buffer,
                              const vk::raii::CommandBuffer& command_buffer,
                              const std::string& name, i32 index) {
  gpu::Buffer buffer{allocator_, buffer_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eBuffer,
                reinterpret_cast<uint64_t>(static_cast<VkBuffer>(*buffer)),
                name, "Buffer", index == -1 ? "" : std::to_string(index));
#endif
  if (*staging_buffer) {
    vk::BufferCopy buffer_copy{0, 0, buffer_ci.size};
    command_buffer.copyBuffer(*staging_buffer, *buffer, buffer_copy);
  }

  return buffer;
}

gpu::Image Gpu::CreateImage(const vk::ImageCreateInfo& image_ci,
                            const vk::ImageLayout& new_layout,
                            const vk::raii::CommandBuffer& command_buffer,
                            const std::string& name, i32 index) {
  gpu::Image image{allocator_, image_ci};

#ifndef NDEBUG
  std::string prefix;
  if (image_ci.imageType == vk::ImageType::e1D) {
    prefix = "1D Image";
  } else if (image_ci.imageType == vk::ImageType::e2D) {
    prefix = "2D Image";
  } else if (image_ci.imageType == vk::ImageType::e3D) {
    prefix = "3D Image";
  }

  SetObjectName(vk::ObjectType::eImage,
                reinterpret_cast<u64>(static_cast<VkImage>(*image)), name,
                prefix, index == -1 ? "" : std::to_string(index));
#endif

  if (new_layout != vk::ImageLayout::eUndefined) {
    vk::ImageMemoryBarrier barrier{
        {},
        vk::AccessFlagBits::eShaderWrite,
        image_ci.initialLayout,
        new_layout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *image,
        {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
         VK_REMAINING_ARRAY_LAYERS}};
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eAllCommands, {},
                                   {}, {}, barrier);
  }

  return image;
}

gpu::Image Gpu::CreateImage(const vk::ImageCreateInfo& image_ci,
                            const vk::ImageLayout& new_layout,
                            const gpu::Buffer& staging_buffer,
                            const vk::raii::CommandBuffer& command_buffer,
                            const tinygltf::Image& tinygltf_image,
                            const std::string& name, i32 index) {
  gpu::Image image{allocator_, image_ci};

#ifndef NDEBUG
  std::string prefix;
  if (image_ci.imageType == vk::ImageType::e1D) {
    prefix = "1D Image";
  } else if (image_ci.imageType == vk::ImageType::e2D) {
    prefix = "2D Image";
  } else if (image_ci.imageType == vk::ImageType::e3D) {
    prefix = "3D Image";
  }

  SetObjectName(vk::ObjectType::eImage,
                reinterpret_cast<u64>(static_cast<VkImage>(*image)), name,
                prefix, index == -1 ? "" : std::to_string(index));
#endif
  vk::ImageAspectFlagBits flag_bits{};
  if (image_ci.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
    flag_bits = vk::ImageAspectFlagBits::eDepth;
  } else {
    flag_bits = vk::ImageAspectFlagBits::eColor;
  }
  if (*staging_buffer) {
    {
      vk::ImageMemoryBarrier barrier{{},
                                     vk::AccessFlagBits::eTransferWrite,
                                     image_ci.initialLayout,
                                     vk::ImageLayout::eTransferDstOptimal,
                                     VK_QUEUE_FAMILY_IGNORED,
                                     VK_QUEUE_FAMILY_IGNORED,
                                     *image,
                                     {flag_bits, 0, VK_REMAINING_MIP_LEVELS, 0,
                                      VK_REMAINING_ARRAY_LAYERS}};

      command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                     vk::PipelineStageFlagBits::eTransfer, {},
                                     {}, {}, barrier);
    }

    {
      const std::vector<vk::Extent3D>& mipmap_extents{
          {static_cast<u32>(tinygltf_image.width),
           static_cast<u32>(tinygltf_image.height), 1}};
      u32 layer_count{1};

      std::vector<vk::BufferImageCopy> buffer_image_copys;
      for (u32 i{}; i < layer_count; ++i) {
        vk::BufferImageCopy buffer_image_copy{
            0, {}, {}, {flag_bits, i, 0, layer_count}, {}, mipmap_extents[i]};
        buffer_image_copys.push_back(buffer_image_copy);
      }
      command_buffer.copyBufferToImage(*staging_buffer, *image,
                                       vk::ImageLayout::eTransferDstOptimal,
                                       buffer_image_copys);
    }

    {
      vk::ImageMemoryBarrier barrier{vk::AccessFlagBits::eTransferWrite,
                                     vk::AccessFlagBits::eShaderRead,
                                     vk::ImageLayout::eTransferDstOptimal,
                                     new_layout,
                                     VK_QUEUE_FAMILY_IGNORED,
                                     VK_QUEUE_FAMILY_IGNORED,
                                     *image,
                                     {flag_bits, 0, VK_REMAINING_MIP_LEVELS, 0,
                                      VK_REMAINING_ARRAY_LAYERS}};
      command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                     vk::PipelineStageFlagBits::eAllCommands,
                                     {}, {}, {}, barrier);
    }

  } else if (*command_buffer) {
    vk::ImageMemoryBarrier barrier{
        {},
        vk::AccessFlagBits::eShaderRead,
        image_ci.initialLayout,
        new_layout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *image,
        {flag_bits, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS}};
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eAllCommands, {},
                                   {}, {}, barrier);
  }

  return image;
}

vk::raii::ImageView Gpu::CreateImageView(
    const vk::ImageViewCreateInfo& image_view_ci, const std::string& name,
    i32 index) {
  vk::raii::ImageView image_view{vk::raii::ImageView{device_, image_view_ci}};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eImageView,
                reinterpret_cast<u64>(static_cast<VkImageView>(*image_view)),
                name, "Image View", index == -1 ? "" : std::to_string(index));
#endif

  return image_view;
}

vk::raii::Sampler Gpu::CreateSampler(const vk::SamplerCreateInfo& sampler_ci,
                                     const std::string& name, i32 index) {
  vk::raii::Sampler sampler{device_, sampler_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eSampler,
                reinterpret_cast<uint64_t>(static_cast<VkSampler>(*sampler)),
                name, "Sampler", index == -1 ? "" : std::to_string(index));
#endif

  return sampler;
}

vk::raii::SwapchainKHR Gpu::CreateSwapchain(
    vk::SwapchainCreateInfoKHR swapchain_ci, const std::string& name,
    i32 index) {
  swapchain_ci.surface = *surface_;
  vk::raii::SwapchainKHR swapchain{device_, swapchain_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eSwapchainKHR,
      reinterpret_cast<uint64_t>(static_cast<VkSwapchainKHR>(*swapchain)), name,
      "Swapchain", index == -1 ? "" : std::to_string(index));
#endif

  return swapchain;
}

vk::raii::Semaphore Gpu::CreateSemaphoreLuka(
    const vk::SemaphoreCreateInfo& semaphore_ci, const std::string& name,
    i32 index) {
  vk::raii::Semaphore semaphore{device_, semaphore_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eSemaphore,
      reinterpret_cast<uint64_t>(static_cast<VkSemaphore>(*semaphore)), name,
      "Semaphore", index == -1 ? "" : std::to_string(index));
#endif

  return semaphore;
}

vk::raii::Fence Gpu::CreateFence(const vk::FenceCreateInfo& fence_ci,
                                 const std::string& name, i32 index) {
  vk::raii::Fence fence{device_, fence_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eFence,
                reinterpret_cast<uint64_t>(static_cast<VkFence>(*fence)), name,
                "Fence", index == -1 ? "" : std::to_string(index));
#endif

  return fence;
}

vk::raii::CommandPool Gpu::CreateCommandPool(
    const vk::CommandPoolCreateInfo& command_pool_ci, const std::string& name,
    i32 index) {
  vk::raii::CommandPool command_pool{device_, command_pool_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eCommandPool,
      reinterpret_cast<uint64_t>(static_cast<VkCommandPool>(*command_pool)),
      name, "Command Pool", index == -1 ? "" : std::to_string(index));
#endif

  return command_pool;
}

vk::raii::RenderPass Gpu::CreateRenderPass(
    const vk::RenderPassCreateInfo& render_pass_ci, const std::string& name,
    i32 index) {
  vk::raii::RenderPass render_pass{device_, render_pass_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eRenderPass,
      reinterpret_cast<uint64_t>(static_cast<VkRenderPass>(*render_pass)), name,
      "Render Pass", index == -1 ? "" : std::to_string(index));
#endif

  return render_pass;
}

vk::raii::Framebuffer Gpu::CreateFramebuffer(
    const vk::FramebufferCreateInfo& framebuffer_ci, const std::string& name,
    i32 index) {
  vk::raii::Framebuffer framebuffer{device_, framebuffer_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eFramebuffer,
      reinterpret_cast<uint64_t>(static_cast<VkFramebuffer>(*framebuffer)),
      name, "Framebuffer", index == -1 ? "" : std::to_string(index));
#endif

  return framebuffer;
}

vk::raii::DescriptorPool Gpu::CreateDescriptorPool(
    const vk::DescriptorPoolCreateInfo& descriptor_pool_ci,
    const std::string& name, i32 index) {
  vk::raii::DescriptorPool descriptor_pool{device_, descriptor_pool_ci};
#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eDescriptorPool,
                reinterpret_cast<uint64_t>(
                    static_cast<VkDescriptorPool>(*descriptor_pool)),
                name, "Descriptor Pool",
                index == -1 ? "" : std::to_string(index));
#endif
  return descriptor_pool;
}

vk::raii::DescriptorSetLayout Gpu::CreateDescriptorSetLayout(
    const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
    const std::string& name, i32 index) {
  vk::raii::DescriptorSetLayout descriptor_set_layout{device_,
                                                      descriptor_set_layout_ci};
#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eDescriptorSetLayout,
                reinterpret_cast<uint64_t>(
                    static_cast<VkDescriptorSetLayout>(*descriptor_set_layout)),
                name, "Descriptor Layout",
                index == -1 ? "" : std::to_string(index));
#endif

  return descriptor_set_layout;
}

vk::raii::PipelineLayout Gpu::CreatePipelineLayout(
    const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
    const std::string& name, i32 index) {
  vk::raii::PipelineLayout pipeline_layout{device_, pipeline_layout_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::ePipelineLayout,
                reinterpret_cast<uint64_t>(
                    static_cast<VkPipelineLayout>(*pipeline_layout)),
                name, "Pipeline Layout",
                index == -1 ? "" : std::to_string(index));
#endif
  return pipeline_layout;
}

vk::raii::ShaderModule Gpu::CreateShaderModule(
    const vk::ShaderModuleCreateInfo& shader_module_ci, const std::string& name,
    i32 index) {
  vk::raii::ShaderModule shader_module{device_, shader_module_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eShaderModule,
      reinterpret_cast<uint64_t>(static_cast<VkShaderModule>(*shader_module)),
      name, "Shader Module", index == -1 ? "" : std::to_string(index));
#endif
  return shader_module;
}

vk::raii::PipelineCache Gpu::CreatePipelineCache(
    const vk::PipelineCacheCreateInfo& pipeline_cache_ci,
    const std::string& name, i32 index) {
  vk::raii::PipelineCache pipeline_cache{device_, pipeline_cache_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::ePipelineCache,
      reinterpret_cast<uint64_t>(static_cast<VkPipelineCache>(*pipeline_cache)),
      name, "Pipeline Cache", index == -1 ? "" : std::to_string(index));
#endif
  return pipeline_cache;
}

vk::raii::Pipeline Gpu::CreatePipeline(
    const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci,
    const vk::raii::PipelineCache& pipeline_cache, const std::string& name,
    i32 index) {
  vk::raii::Pipeline pipeline{device_, pipeline_cache, graphics_pipeline_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::ePipeline,
                reinterpret_cast<uint64_t>(static_cast<VkPipeline>(*pipeline)),
                name, "Graphics Pipeline",
                index == -1 ? "" : std::to_string(index));
#endif

  return pipeline;
}

vk::raii::Pipeline Gpu::CreatePipeline(
    const vk::ComputePipelineCreateInfo& compute_pipeline_ci,
    const vk::raii::PipelineCache& pipeline_cache, const std::string& name,
    i32 index) {
  vk::raii::Pipeline pipeline{device_, pipeline_cache, compute_pipeline_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::ePipeline,
                reinterpret_cast<uint64_t>(static_cast<VkPipeline>(*pipeline)),
                name, "Graphics Pipeline",
                index == -1 ? "" : std::to_string(index));
#endif

  return pipeline;
}

vk::raii::CommandBuffers Gpu::AllocateCommandBuffers(
    const vk::CommandBufferAllocateInfo& command_buffer_ai,
    const std::string& name, i32 index) {
  vk::raii::CommandBuffers command_buffers{device_, command_buffer_ai};

#ifndef NDEBUG
  for (u32 i{}; i < command_buffers.size(); ++i) {
    SetObjectName(vk::ObjectType::eCommandBuffer,
                  reinterpret_cast<uint64_t>(
                      static_cast<VkCommandBuffer>(*command_buffers[i])),
                  name, "Command Buffer",
                  index == -1 ? "" : std::to_string(index));
  }
#endif

  return command_buffers;
}

vk::raii::DescriptorSets Gpu::AllocateNormalDescriptorSets(
    vk::DescriptorSetAllocateInfo descriptor_set_ai, const std::string& name,
    i32 index) {
  descriptor_set_ai.descriptorPool = *normal_descriptor_pool_;
  vk::raii::DescriptorSets descriptor_sets{device_, descriptor_set_ai};

#ifndef NDEBUG
  for (u32 i{}; i < descriptor_sets.size(); ++i) {
    SetObjectName(vk::ObjectType::eDescriptorSet,
                  reinterpret_cast<uint64_t>(
                      static_cast<VkDescriptorSet>(*(descriptor_sets[i]))),
                  name, "Descriptor Set",
                  index == -1 ? "" : std::to_string(index));
  }

#endif

  return descriptor_sets;
}

vk::raii::DescriptorSet Gpu::AllocateBindlessDescriptorSet(
    vk::DescriptorSetAllocateInfo descriptor_set_ai, const std::string& name,
    i32 index) {
  if (descriptor_set_ai.descriptorSetCount != 1) {
    LOGW("Descriptor set count is not 1");
  }
  descriptor_set_ai.descriptorPool = *bindless_descriptor_pool_;
  vk::raii::DescriptorSets descriptor_sets{device_, descriptor_set_ai};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eDescriptorSet,
                reinterpret_cast<uint64_t>(
                    static_cast<VkDescriptorSet>(*(descriptor_sets[0]))),
                name, "Descriptor Set",
                index == -1 ? "" : std::to_string(index));
#endif

  return std::move(descriptor_sets[0]);
}

void Gpu::UpdateDescriptorSets(
    const std::vector<vk::WriteDescriptorSet>& writes) {
  device_.updateDescriptorSets(writes, nullptr);
}

vk::Result Gpu::WaitSemaphores(const vk::SemaphoreWaitInfo& semaphore_wi) {
  return device_.waitSemaphores(semaphore_wi, UINT64_MAX);
}

vk::Result Gpu::WaitForFences(const vk::raii::Fence& fence) {
  return device_.waitForFences(*fence, VK_TRUE, UINT64_MAX);
}

void Gpu::ResetFence(const vk::raii::Fence& fence) {
  device_.resetFences(*fence);
}

void Gpu::WaitIdle() { device_.waitIdle(); }

void Gpu::GraphicsQueueSubmit2(const vk::SubmitInfo2& submit_info) {
  graphics_queue_.submit2(submit_info);
}

void Gpu::ComputeQueueSubmit2(const vk::SubmitInfo2& submit_info) {
  compute_queue_.submit2(submit_info);
}

void Gpu::TransferQueueSubmit(const vk::SubmitInfo& submit_info) {
  transfer_queue_.submit(submit_info);
  transfer_queue_.waitIdle();
}

vk::Result Gpu::PresentQueuePresent(const vk::PresentInfoKHR& present_info) {
  return present_queue_.presentKHR(present_info);
}

void Gpu::BeginLabel(const vk::raii::CommandBuffer& command_buffer,
                     const std::string& label,
                     const std::array<f32, 4>& color) {
  vk::DebugUtilsLabelEXT debug_utils_label{label.c_str(), color};
  command_buffer.beginDebugUtilsLabelEXT(debug_utils_label);
}

void Gpu::EndLabel(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.endDebugUtilsLabelEXT();
}

void Gpu::CreateInstance() {
  vk::InstanceCreateFlags flags;
#ifdef __APPLE__
  flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

  vk::ApplicationInfo application_info{"luka", VK_MAKE_VERSION(1, 0, 0), "luka",
                                       VK_MAKE_VERSION(1, 0, 0),
                                       VK_API_VERSION_1_2};

  std::unordered_map<std::string, bool> requested_instance_layers;
  std::unordered_map<std::string, bool> requested_instance_extensions;

  std::vector<const char*> window_required_instance_extensions{
      window_->GetRequiredInstanceExtensions()};

  for (const char* wrie : window_required_instance_extensions) {
    requested_instance_extensions.emplace(wrie, true);
  }

#ifdef __APPLE__
  requested_instance_extensions.emplace(
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, true);
  requested_instance_extensions.emplace(
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, true);
#endif

#ifndef NDEBUG
  requested_instance_layers.emplace("VK_LAYER_KHRONOS_validation", false);
  requested_instance_extensions.emplace(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                        false);
#endif

  std::vector<vk::LayerProperties> instance_layer_properties{
      context_.enumerateInstanceLayerProperties()};
  std::vector<vk::ExtensionProperties> instance_extension_properties{
      context_.enumerateInstanceExtensionProperties()};

  for (const vk::LayerProperties& ilp : instance_layer_properties) {
    if (requested_instance_layers.find(ilp.layerName) !=
        requested_instance_layers.end()) {
      enabled_instance_layers_.insert(ilp.layerName);
    }
  }

  for (const vk::ExtensionProperties& iep : instance_extension_properties) {
    if (requested_instance_extensions.find(iep.extensionName) !=
        requested_instance_extensions.end()) {
      enabled_instance_extensions_.insert(iep.extensionName);
    }
  }

  for (const auto& ril : requested_instance_layers) {
    if (enabled_instance_layers_.find(ril.first) ==
        enabled_instance_layers_.end()) {
      if (ril.second) {
        THROW("Fail to find required instance layer: {}", ril.first);
      }
      LOGI("Not find optional instance layer: {}", ril.first);
    }
  }

  for (const auto& rie : requested_instance_extensions) {
    if (enabled_instance_extensions_.find(rie.first) ==
        enabled_instance_extensions_.end()) {
      if (rie.second) {
        THROW("Fail to find required instance extenion: {}", rie.first);
      }
      LOGI("Not find optional instance extenion: {}", rie.first);
    }
  }

  std::vector<const char*> enabled_layers;
  std::vector<const char*> enabled_extensions;
  enabled_layers.reserve(enabled_instance_layers_.size());
  enabled_extensions.reserve(enabled_instance_extensions_.size());
  for (const std::string& eil : enabled_instance_layers_) {
    enabled_layers.push_back(eil.c_str());
  }
  for (const std::string& eie : enabled_instance_extensions_) {
    enabled_extensions.push_back(eie.c_str());
  }

  vk::InstanceCreateInfo instance_ci{flags, &application_info, enabled_layers,
                                     enabled_extensions};

#ifndef NDEBUG
  vk::DebugUtilsMessageSeverityFlagsEXT message_severity_flags{
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError};

  vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation};

  vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_ci{
      {},
      message_severity_flags,
      message_type_flags,
      &DebugUtilsMessengerCallback};

  vk::StructureChain<vk::InstanceCreateInfo,
                     vk::DebugUtilsMessengerCreateInfoEXT>
      instance_chain{instance_ci, debug_utils_messenger_ci};

  instance_ = vk::raii::Instance{context_,
                                 instance_chain.get<vk::InstanceCreateInfo>()};

  debug_utils_messenger_ = vk::raii::DebugUtilsMessengerEXT{
      instance_, instance_chain.get<vk::DebugUtilsMessengerCreateInfoEXT>()};
#else
  instance_ = vk::raii::Instance{context_, instance_ci};
#endif
}

void Gpu::CreateSurface() {
  VkSurfaceKHR surface{};
  window_->CreateWindowSurface(instance_, &surface);
  surface_ = vk::raii::SurfaceKHR{instance_, surface};
}

void Gpu::CreatePhysicalDevice() {
  vk::raii::PhysicalDevices physical_devices{instance_};

  u32 max_score{};
  for (auto& physical_device : physical_devices) {
    u32 cur_score{1};

    vk::PhysicalDeviceProperties physical_device_properties{
        physical_device.getProperties()};
    switch (physical_device_properties.deviceType) {
      case vk::PhysicalDeviceType::eDiscreteGpu:
        cur_score += 10000;
        break;
      case vk::PhysicalDeviceType::eIntegratedGpu:
        cur_score += 1000;
        break;
      case vk::PhysicalDeviceType::eVirtualGpu:
        cur_score += 100;
        break;
      case vk::PhysicalDeviceType::eCpu:
        cur_score += 10;
        break;
      default:
        break;
    }

    if (cur_score > max_score) {
      max_score = cur_score;
      physical_device_ = std::move(physical_device);
    }
  }

  if (!(*physical_device_)) {
    THROW("Fail to find physical device.");
  }
}

void Gpu::CreateDevice() {
  // Queue famliy properties.
  std::vector<vk::QueueFamilyProperties> queue_family_properties{
      physical_device_.getQueueFamilyProperties()};
  u32 i{};
  for (const auto& queue_famliy_propertie : queue_family_properties) {
    if ((queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eGraphics) &&
        (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eCompute) &&
        (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eTransfer) &&
        physical_device_.getSurfaceSupportKHR(i, *surface_)) {
      graphics_queue_index_ = i;
      compute_queue_index_ = i;
      transfer_queue_index_ = i;
      present_queue_index_ = i;
      break;
    }
    ++i;
  }
  if (!(graphics_queue_index_.has_value() && compute_queue_index_.has_value() &&
        present_queue_index_.has_value())) {
    i = 0;
    for (const auto& queue_famliy_propertie : queue_family_properties) {
      if (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eGraphics) {
        graphics_queue_index_ = i;
      }
      if (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eCompute) {
        compute_queue_index_ = i;
      }
      if (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eTransfer) {
        transfer_queue_index_ = i;
      }
      if (physical_device_.getSurfaceSupportKHR(i, *surface_)) {
        present_queue_index_ = i;
      }
      if (graphics_queue_index_.has_value() &&
          compute_queue_index_.has_value() &&
          transfer_queue_index_.has_value() &&
          present_queue_index_.has_value()) {
        break;
      }
      ++i;
    }
  }

  if (!(graphics_queue_index_.has_value() && compute_queue_index_.has_value() &&
        transfer_queue_index_.has_value() &&
        present_queue_index_.has_value())) {
    THROW("Fail to find queue family.");
  }

  std::vector<vk::DeviceQueueCreateInfo> device_queue_cis;
  std::set<u32> queue_family_indexes{
      graphics_queue_index_.value(), compute_queue_index_.value(),
      transfer_queue_index_.value(), present_queue_index_.value()};

  f32 queue_priority{};
  for (u32 queue_family_index : queue_family_indexes) {
    vk::DeviceQueueCreateInfo device_queue_ci{
        {}, queue_family_index, 1, &queue_priority};
    device_queue_cis.push_back(device_queue_ci);
  }

  // Extensions.
  std::vector<vk::ExtensionProperties> device_extension_properties{
      physical_device_.enumerateDeviceExtensionProperties()};

  std::unordered_map<std::string, bool> requested_device_extensions{
      {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true},
      {VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, true},
      {VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME, false}};

#ifdef __APPLE__
  requested_device_extensions.emplace("VK_KHR_portability_subset", true);
#endif

  for (const vk::ExtensionProperties& dep : device_extension_properties) {
    if (requested_device_extensions.find(dep.extensionName) !=
        requested_device_extensions.end()) {
      enabled_device_extensions_.insert(dep.extensionName);
    }
  }

  for (const auto& rde : requested_device_extensions) {
    if (enabled_device_extensions_.find(rde.first) ==
        enabled_device_extensions_.end()) {
      if (rde.second) {
        THROW("Fail to find required device extenion: {}", rde.first);
      }
      LOGI("Not find optional device extenion: {}", rde.first);
    }
  }

  std::vector<const char*> enabled_extensions;
  enabled_extensions.reserve(enabled_device_extensions_.size());
  for (const std::string& ede : enabled_device_extensions_) {
    enabled_extensions.push_back(ede.c_str());
  }

  // Features.
  void* next_feature{};

  auto features_chain{physical_device_.getFeatures2<
      vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features,
      vk::PhysicalDeviceSynchronization2FeaturesKHR,
      vk::PhysicalDeviceIndexTypeUint8FeaturesEXT>()};

  vk::PhysicalDeviceVulkan12Features enabled_vulkan12_features;
  const auto& vulkan12_features{
      features_chain.get<vk::PhysicalDeviceVulkan12Features>()};
  if (vulkan12_features.shaderSampledImageArrayNonUniformIndexing &&
      vulkan12_features.descriptorBindingPartiallyBound &&
      vulkan12_features.runtimeDescriptorArray &&
      vulkan12_features.timelineSemaphore) {
    enabled_vulkan12_features.pNext = next_feature;
    next_feature = &enabled_vulkan12_features;

    enabled_vulkan12_features.shaderSampledImageArrayNonUniformIndexing =
        VK_TRUE;
    enabled_vulkan12_features.descriptorBindingPartiallyBound = VK_TRUE;
    enabled_vulkan12_features.runtimeDescriptorArray = VK_TRUE;
    enabled_vulkan12_features.timelineSemaphore = VK_TRUE;
  } else {
    THROW("Fail to enable required vulkan12 features");
  }

  vk::PhysicalDeviceSynchronization2FeaturesKHR
      enabled_synchronization2_features;
  const auto& synchronization2_features{
      features_chain.get<vk::PhysicalDeviceSynchronization2FeaturesKHR>()};
  if (synchronization2_features.synchronization2) {
    enabled_synchronization2_features.pNext = next_feature;
    next_feature = &enabled_synchronization2_features;

    enabled_synchronization2_features.synchronization2 = VK_TRUE;
  } else {
    THROW("Fail to enable required synchronization2 features");
  }

  vk::PhysicalDeviceIndexTypeUint8FeaturesEXT enabled_index_type_uint8_features;
  if (enabled_device_extensions_.find(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME) !=
      enabled_device_extensions_.end()) {
    const auto& index_type_uint8_features{
        features_chain.get<vk::PhysicalDeviceIndexTypeUint8FeaturesEXT>()};
    if (index_type_uint8_features.indexTypeUint8) {
      enabled_index_type_uint8_features.pNext = next_feature;
      next_feature = &enabled_index_type_uint8_features;

      enabled_index_type_uint8_features.indexTypeUint8 = VK_TRUE;

      has_index_type_uint8_ = true;
    } else {
      has_index_type_uint8_ = false;
      LOGI("Not support optional index type uint8 features");
    }
  }

  vk::PhysicalDeviceFeatures enabled_features;
  const auto& features{
      features_chain.get<vk::PhysicalDeviceFeatures2>().features};

  vk::PhysicalDeviceFeatures2 enabled_features2{enabled_features, next_feature};

  // Create device.
  vk::DeviceCreateInfo device_ci{{},      device_queue_cis,
                                 {},      enabled_extensions,
                                 nullptr, &enabled_features2};

  device_ = vk::raii::Device{physical_device_, device_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eDevice,
                reinterpret_cast<uint64_t>(static_cast<VkDevice>(*device_)),
                "luka", "Device");
#endif

  // Create queue.
  graphics_queue_ = vk::raii::Queue{device_, graphics_queue_index_.value(), 0};
  compute_queue_ = vk::raii::Queue{device_, compute_queue_index_.value(), 0};
  transfer_queue_ = vk::raii::Queue{device_, transfer_queue_index_.value(), 0};
  present_queue_ = vk::raii::Queue{device_, present_queue_index_.value(), 0};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eQueue,
      reinterpret_cast<uint64_t>(static_cast<VkQueue>(*graphics_queue_)),
      "graphics", "Queue");
  SetObjectName(
      vk::ObjectType::eQueue,
      reinterpret_cast<uint64_t>(static_cast<VkQueue>(*compute_queue_)),
      "compute", "Queue");
  SetObjectName(
      vk::ObjectType::eQueue,
      reinterpret_cast<uint64_t>(static_cast<VkQueue>(*transfer_queue_)),
      "transfer", "Queue");
  SetObjectName(
      vk::ObjectType::eQueue,
      reinterpret_cast<uint64_t>(static_cast<VkQueue>(*present_queue_)),
      "present", "Queue");
#endif
}

void Gpu::CreateVmaAllocator() {
  VmaAllocatorCreateInfo allocator_ci{
      .flags = 0,
      .physicalDevice = static_cast<VkPhysicalDevice>(*physical_device_),
      .device = static_cast<VkDevice>(*device_),
      .instance = static_cast<VkInstance>(*instance_),
      .vulkanApiVersion = VK_API_VERSION_1_2,
  };
  vmaCreateAllocator(&allocator_ci, &allocator_);
}

void Gpu::CreateDescriptorPool() {
  // Bindless.
  std::vector<vk::DescriptorPoolSize> bindlessl_pool_sizes{
      {vk::DescriptorType::eSampler, 128},
      {vk::DescriptorType::eSampledImage, 1024}};

  u32 bindless_max_sets{std::accumulate(
      bindlessl_pool_sizes.begin(), bindlessl_pool_sizes.end(),
      static_cast<u32>(0), [](u32 sum, const vk::DescriptorPoolSize& dps) {
        return sum + dps.descriptorCount;
      })};

  vk::DescriptorPoolCreateInfo bindless_descriptor_pool_ci{
      vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind |
          vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      bindless_max_sets, bindlessl_pool_sizes};
  bindless_descriptor_pool_ =
      CreateDescriptorPool(bindless_descriptor_pool_ci, "bindless");

  // Normal.
  std::vector<vk::DescriptorPoolSize> normal_pool_sizes{
      {vk::DescriptorType::eSampler, 1024},
      {vk::DescriptorType::eCombinedImageSampler, 1024},
      {vk::DescriptorType::eSampledImage, 1024},
      {vk::DescriptorType::eStorageImage, 1024},
      {vk::DescriptorType::eUniformTexelBuffer, 1024},
      {vk::DescriptorType::eStorageTexelBuffer, 1024},
      {vk::DescriptorType::eUniformBuffer, 1024},
      {vk::DescriptorType::eStorageBuffer, 1024},
      {vk::DescriptorType::eUniformBufferDynamic, 1024},
      {vk::DescriptorType::eStorageBufferDynamic, 1024},
      {vk::DescriptorType::eInputAttachment, 1024}};

  u32 normal_max_sets{std::accumulate(
      normal_pool_sizes.begin(), normal_pool_sizes.end(), static_cast<u32>(0),
      [](u32 sum, const vk::DescriptorPoolSize& dps) {
        return sum + dps.descriptorCount;
      })};

  vk::DescriptorPoolCreateInfo normal_descriptor_pool_ci{
      vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, normal_max_sets,
      normal_pool_sizes};
  normal_descriptor_pool_ =
      CreateDescriptorPool(normal_descriptor_pool_ci, "normal");
}

void Gpu::CreateDefaultResource() {
  vk::SamplerCreateInfo sampler_ci{vk::SamplerCreateFlags(),
                                   vk::Filter::eLinear,
                                   vk::Filter::eLinear,
                                   vk::SamplerMipmapMode::eLinear,
                                   vk::SamplerAddressMode::eRepeat,
                                   vk::SamplerAddressMode::eRepeat,
                                   vk::SamplerAddressMode::eRepeat,
                                   0.0F,
                                   VK_FALSE,
                                   1.0F,
                                   VK_FALSE,
                                   vk::CompareOp::eAlways,
                                   0.0F,
                                   0.0F,
                                   vk::BorderColor::eFloatTransparentBlack,
                                   VK_FALSE};

  sampler_ = CreateSampler(sampler_ci, "default");
}

void Gpu::DestroyAllocator() { vmaDestroyAllocator(allocator_); }

void Gpu::SetObjectName(vk::ObjectType object_type, u64 handle,
                        const std::string& name, const std::string& prefix,
                        const std::string& suffix) {
  if (name.empty()) {
    return;
  }

  std::string object_name{name};
  if (!prefix.empty()) {
    object_name = prefix + " " + object_name;
  }
  if (!suffix.empty()) {
    object_name = object_name + " " + suffix;
  }

  vk::DebugUtilsObjectNameInfoEXT object_name_info{object_type, handle,
                                                   object_name.c_str()};
  device_.setDebugUtilsObjectNameEXT(object_name_info);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Gpu::DebugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity_flag_bits,
    VkDebugUtilsMessageTypeFlagsEXT message_type_flags,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* /*p_user_data*/) {
  std::string message_severity{
      vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(
          message_severity_flag_bits))};
  std::string message_type{vk::to_string(
      static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_type_flags))};

  LOGI("{} : {}", message_severity, message_type);
  LOGI("\tMessage id name   = [{}]", p_callback_data->pMessageIdName);
  LOGI("\tMessage id number = [{}]", p_callback_data->messageIdNumber);
  LOGI("\tMessage           = [{}]", p_callback_data->pMessage);
  if (p_callback_data->queueLabelCount > 0) {
    LOGI("\tQueue lables:");
    for (u32 i{}; i < p_callback_data->queueLabelCount; i++) {
      LOGI("\t\tLabel name = [{}]",
           p_callback_data->pQueueLabels[i].pLabelName);
    }
  }
  if (p_callback_data->cmdBufLabelCount > 0) {
    LOGI("\tCommand buffer labels:");
    for (u32 i{}; i < p_callback_data->cmdBufLabelCount; i++) {
      LOGI("\t\tLabel name = [{}]",
           p_callback_data->pCmdBufLabels[i].pLabelName);
    }
  }
  if (p_callback_data->objectCount > 0) {
    LOGI("\tObjects:");
    for (u32 i{}; i < p_callback_data->objectCount; i++) {
      LOGI("\t\tObject {}", i);
      LOGI("\t\t\tObject type   = {}",
           vk::to_string(static_cast<vk::ObjectType>(
               p_callback_data->pObjects[i].objectType)));
      LOGI("\t\t\tObject handle = {}",
           p_callback_data->pObjects[i].objectHandle);
      if (p_callback_data->pObjects[i].pObjectName) {
        LOGI("\t\t\tObject name   = {}",
             p_callback_data->pObjects[i].pObjectName);
      }
    }
  }
  return VK_TRUE;
}

}  // namespace luka
