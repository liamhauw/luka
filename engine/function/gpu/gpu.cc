// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"

#include <vulkan/vulkan_hash.hpp>

#include "core/log.h"
#include "core/util.h"

namespace luka {

Gpu::Gpu(std::shared_ptr<Window> window) : window_{window} {
  CreateInstance();
  CreateSurface();
  CreatePhysicalDevice();
  CreateDevice();
  CreateAllocator();
  CreateCommandObjects();
  CreateDescriptorPool();
}

Gpu::~Gpu() {
  WaitIdle();
  vmaDestroyAllocator(allocator_);
}

void Gpu::Tick() {}

gpu::Buffer Gpu::CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                              const void* data, bool map,
                              const std::string& name) {
  gpu::Buffer buffer{allocator_, buffer_ci, true};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eBuffer,
                reinterpret_cast<uint64_t>(static_cast<VkBuffer>(*buffer)),
                name, "Buffer");
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
                              const std::string& name) {
  gpu::Buffer buffer{allocator_, buffer_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eBuffer,
                reinterpret_cast<uint64_t>(static_cast<VkBuffer>(*buffer)),
                name, "Buffer");
#endif
  if (*staging_buffer) {
    vk::BufferCopy buffer_copy{0, 0, buffer_ci.size};
    command_buffer.copyBuffer(*staging_buffer, *buffer, buffer_copy);
  }

  return buffer;
}

gpu::Image Gpu::CreateImage(const vk::ImageCreateInfo& image_ci,
                            const std::string& name) {
  gpu::Image image{allocator_, image_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eImage,
                reinterpret_cast<u64>(static_cast<VkImage>(*image)), name,
                "Image");
#endif

  return image;
}

gpu::Image Gpu::CreateImage(const vk::ImageCreateInfo& image_ci,
                            const vk::ImageLayout new_layout,
                            const gpu::Buffer& staging_buffer,
                            const vk::raii::CommandBuffer& command_buffer,
                            const ast::Image& asset_image,
                            const std::string& name) {
  gpu::Image image{allocator_, image_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eImage,
                reinterpret_cast<u64>(static_cast<VkImage>(*image)), name,
                "Image");
#endif
  vk::ImageAspectFlagBits flag_bits;
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
          asset_image.GetMipmapExtents()};
      u32 layer_count{asset_image.GetLayerCount()};
      const auto& offsets{asset_image.GetOffsets()};

      std::vector<vk::BufferImageCopy> buffer_image_copys;
      for (u32 i{0}; i < layer_count; ++i) {
        vk::BufferImageCopy buffer_image_copy{
            offsets.empty() ? 0 : offsets[i][0][0], {}, {},
            {flag_bits, i, 0, layer_count},         {}, mipmap_extents[i]};
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
    const vk::ImageViewCreateInfo& image_view_ci, const std::string& name) {
  vk::raii::ImageView image_view{vk::raii::ImageView{device_, image_view_ci}};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eImageView,
                reinterpret_cast<u64>(static_cast<VkImageView>(*image_view)),
                name, "Image View");
#endif

  return image_view;
}

vk::raii::Sampler Gpu::CreateSampler(const vk::SamplerCreateInfo sampler_ci,
                                     const std::string& name) {
  vk::raii::Sampler sampler{device_, sampler_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eSampler,
                reinterpret_cast<uint64_t>(static_cast<VkSampler>(*sampler)),
                name, "Sampler");
#endif

  return sampler;
}

vk::raii::SwapchainKHR Gpu::CreateSwapchain(
    vk::SwapchainCreateInfoKHR swapchain_ci, const std::string& name) {
  swapchain_ci.surface = *surface_;
  vk::raii::SwapchainKHR swapchain{device_, swapchain_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eSwapchainKHR,
      reinterpret_cast<uint64_t>(static_cast<VkSwapchainKHR>(*swapchain)), name,
      "Swapchain");
#endif

  return swapchain;
}

vk::raii::Semaphore Gpu::CreateSemaphoreLuka(
    const vk::SemaphoreCreateInfo& semaphore_ci, const std::string& name) {
  vk::raii::Semaphore semaphore{device_, semaphore_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eSemaphore,
      reinterpret_cast<uint64_t>(static_cast<VkSemaphore>(*semaphore)), name,
      "Semaphore");
#endif

  return semaphore;
}

vk::raii::Fence Gpu::CreateFence(const vk::FenceCreateInfo& fence_ci,
                                 const std::string& name) {
  vk::raii::Fence fence{device_, fence_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eFence,
                reinterpret_cast<uint64_t>(static_cast<VkFence>(*fence)), name,
                "Fence");
#endif

  return fence;
}

vk::raii::CommandPool Gpu::CreateCommandPool(
    const vk::CommandPoolCreateInfo& command_pool_ci, const std::string& name) {
  vk::raii::CommandPool command_pool{device_, command_pool_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eCommandPool,
      reinterpret_cast<uint64_t>(static_cast<VkCommandPool>(*command_pool)),
      name, "Command_pool");
#endif

  return command_pool;
}

vk::raii::RenderPass Gpu::CreateRenderPass(
    const vk::RenderPassCreateInfo& render_pass_ci, const std::string& name) {
  vk::raii::RenderPass render_pass{device_, render_pass_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eRenderPass,
      reinterpret_cast<uint64_t>(static_cast<VkRenderPass>(*render_pass)), name,
      "Render Pass");
#endif

  return render_pass;
}

vk::raii::Framebuffer Gpu::CreateFramebuffer(
    const vk::FramebufferCreateInfo& framebuffer_ci, const std::string& name) {
  vk::raii::Framebuffer framebuffer{device_, framebuffer_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eFramebuffer,
      reinterpret_cast<uint64_t>(static_cast<VkFramebuffer>(*framebuffer)),
      name, "Framebuffer");
#endif

  return framebuffer;
}

const vk::raii::DescriptorSetLayout& Gpu::RequestDescriptorSetLayout(
    const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
    const std::string& name) {
  u64 hash_value{0};
  for (u32 i{0}; i < descriptor_set_layout_ci.bindingCount; ++i) {
    HashCombine(hash_value, descriptor_set_layout_ci.pBindings[i]);
  }

  std::unordered_map<u64, vk::raii::DescriptorSetLayout>&
      descriptor_set_layouts_{resource_cache_.descriptor_set_layouts};

  auto it{descriptor_set_layouts_.find(hash_value)};
  if (it != descriptor_set_layouts_.end()) {
    return it->second;
  }

  vk::raii::DescriptorSetLayout descriptor_set_layout{device_,
                                                      descriptor_set_layout_ci};
#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eDescriptorSetLayout,
                reinterpret_cast<uint64_t>(
                    static_cast<VkDescriptorSetLayout>(*descriptor_set_layout)),
                name, "Descriptor Set Layout");
#endif

  auto it1{descriptor_set_layouts_.emplace(hash_value,
                                           std::move(descriptor_set_layout))};

  return it1.first->second;
}

const vk::raii::PipelineLayout& Gpu::RequestPipelineLayout(
    const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
    const std::string& name) {
  u64 hash_value{0};
  HashCombine(hash_value, pipeline_layout_ci.flags);
  for (u32 i{0}; i < pipeline_layout_ci.setLayoutCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pSetLayouts[i]);
  }
  for (u32 i{0}; i < pipeline_layout_ci.pushConstantRangeCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pPushConstantRanges[i]);
  }

  std::unordered_map<u64, vk::raii::PipelineLayout>& pipeline_layouts{
      resource_cache_.pipeline_layouts};

  auto it{pipeline_layouts.find(hash_value)};
  if (it != pipeline_layouts.end()) {
    return it->second;
  }

  vk::raii::PipelineLayout pipeline_layout{device_, pipeline_layout_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::ePipelineLayout,
                reinterpret_cast<uint64_t>(
                    static_cast<VkPipelineLayout>(*pipeline_layout)),
                name, "Pipeline Layout");
#endif

  auto it1{pipeline_layouts.emplace(hash_value, std::move(pipeline_layout))};

  return it1.first->second;
}

const vk::raii::ShaderModule& Gpu::RequestShaderModule(
    const vk::ShaderModuleCreateInfo& shader_module_ci,
    const std::string& name) {
  u64 hash_value{0};
  HashCombine(hash_value, shader_module_ci);

  std::unordered_map<u64, vk::raii::ShaderModule>& shader_modules{
      resource_cache_.shader_modules};

  auto it{shader_modules.find(hash_value)};
  if (it != shader_modules.end()) {
    return it->second;
  }

  vk::raii::ShaderModule shader_module{device_, shader_module_ci};

#ifndef NDEBUG
  SetObjectName(
      vk::ObjectType::eShaderModule,
      reinterpret_cast<uint64_t>(static_cast<VkShaderModule>(*shader_module)),
      name, "Shader Module");
#endif

  auto it1{shader_modules.emplace(hash_value, std::move(shader_module))};

  return it1.first->second;
}

const vk::raii::Pipeline& Gpu::RequestPipeline(
    const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci,
    const std::string& name) {
  u64 hash_value{0};
  for (u32 i{0}; i < graphics_pipeline_ci.stageCount; ++i) {
    HashCombine(hash_value, graphics_pipeline_ci.pStages[i]);
  }

  std::unordered_map<u64, vk::raii::Pipeline>& pipelines{
      resource_cache_.pipelines};

  auto it{pipelines.find(hash_value)};
  if (it != pipelines.end()) {
    return it->second;
  }

  vk::raii::Pipeline pipeline{device_, nullptr, graphics_pipeline_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::ePipeline,
                reinterpret_cast<uint64_t>(static_cast<VkPipeline>(*pipeline)),
                name, "Pipeline");
#endif

  auto it1{pipelines.emplace(hash_value, std::move(pipeline))};

  return it1.first->second;
}

vk::raii::CommandBuffers Gpu::AllocateCommandBuffers(
    const vk::CommandBufferAllocateInfo& command_buffer_ai,
    const std::string& name) {
  vk::raii::CommandBuffers command_buffers{device_, command_buffer_ai};

#ifndef NDEBUG
  for (u32 i{0}; i < command_buffers.size(); ++i) {
    SetObjectName(vk::ObjectType::eCommandBuffer,
                  reinterpret_cast<uint64_t>(
                      static_cast<VkCommandBuffer>(*command_buffers[i])),
                  name, "Command Buffer" + std::to_string(i));
  }
#endif

  return command_buffers;
}

vk::raii::DescriptorSets Gpu::AllocateDescriptorSets(
    vk::DescriptorSetAllocateInfo descriptor_set_allocate_info,
    const std::string& name) {
  descriptor_set_allocate_info.descriptorPool = *descriptor_pool_;
  vk::raii::DescriptorSets descriptor_sets{device_,
                                           descriptor_set_allocate_info};

#ifndef NDEBUG
  for (u32 i{0}; i < descriptor_sets.size(); ++i) {
    SetObjectName(vk::ObjectType::eDescriptorSet,
                  reinterpret_cast<uint64_t>(
                      static_cast<VkDescriptorSet>(*(descriptor_sets[i]))),
                  name, "Descriptor Set " + std::to_string(i));
  }

#endif

  return descriptor_sets;
}

void Gpu::UpdateDescriptorSets(
    const std::vector<vk::WriteDescriptorSet>& writes) {
  device_.updateDescriptorSets(writes, nullptr);
}

vk::Result Gpu::WaitForFence(const vk::raii::Fence& fence) {
  return device_.waitForFences(*fence, VK_TRUE, UINT64_MAX);
}

void Gpu::ResetFence(const vk::raii::Fence& fence) {
  device_.resetFences(*fence);
}

const vk::raii::CommandBuffer& Gpu::BeginTempCommandBuffer() {
  const vk::raii::CommandBuffer& command_buffer{command_buffers_[0]};

  vk::CommandBufferBeginInfo command_buffer_begin_info{
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  command_buffer.begin(command_buffer_begin_info);
  return command_buffer;
}

void Gpu::EndTempCommandBuffer(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.end();
  vk::SubmitInfo submit_info{nullptr, nullptr, *command_buffer};
  graphics_queue_.submit(submit_info, nullptr);
  graphics_queue_.waitIdle();
}

void Gpu::WaitIdle() { device_.waitIdle(); }

void Gpu::BeginLabel(const vk::raii::CommandBuffer& command_buffer,
                     const std::string& label,
                     const std::array<f32, 4>& color) {
  vk::DebugUtilsLabelEXT debug_utils_label{label.c_str(), color};
  command_buffer.beginDebugUtilsLabelEXT(debug_utils_label);
}

void Gpu::EndLabel(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.endDebugUtilsLabelEXT();
}

const vk::raii::Queue& Gpu::GetGraphicsQueue() const { return graphics_queue_; }

const vk::raii::Queue& Gpu::GetPresentQueue() const { return present_queue_; }

u32 Gpu::GetGraphicsQueueIndex() const { return graphics_queue_index_.value(); }

u32 Gpu::GetPresentQueueIndex() const { return present_queue_index_.value(); }

vk::SurfaceCapabilitiesKHR Gpu::GetSurfaceCapabilities() const {
  return physical_device_.getSurfaceCapabilitiesKHR(*surface_);
}

std::vector<vk::SurfaceFormatKHR> Gpu::GetSurfaceFormats() const {
  return physical_device_.getSurfaceFormatsKHR(*surface_);
}

std::vector<vk::PresentModeKHR> Gpu::GetSurfacePresentModes() const {
  return physical_device_.getSurfacePresentModesKHR(*surface_);
}

void Gpu::CreateInstance() {
  vk::InstanceCreateFlags flags;
#ifdef __APPLE__
  flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

  vk::ApplicationInfo application_info{"luka", VK_MAKE_VERSION(1, 0, 0), "luka",
                                       VK_MAKE_VERSION(1, 0, 0),
                                       VK_API_VERSION_1_3};

  std::vector<const char*> required_instance_layers;
  std::vector<const char*> required_instance_extensions;

  std::vector<const char*> window_required_instance_extensions{
      window_->GetRequiredInstanceExtensions()};
  required_instance_extensions.insert(
      required_instance_extensions.end(),
      window_required_instance_extensions.begin(),
      window_required_instance_extensions.end());

#ifdef __APPLE__
  required_instance_extensions.push_back(
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  required_instance_extensions.push_back(
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

  const std::vector<vk::LayerProperties> kLayerProperties{
      context_.enumerateInstanceLayerProperties()};
  const std::vector<vk::ExtensionProperties> kExtensionProperties{
      context_.enumerateInstanceExtensionProperties()};

  std::vector<const char*> enabled_instance_layers;
  std::vector<const char*> enabled_instance_extensions;

  enabled_instance_layers.reserve(required_instance_layers.size());
  for (const char* layer : required_instance_layers) {
    if (std::find_if(kLayerProperties.begin(), kLayerProperties.end(),
                     [layer](const vk::LayerProperties& lp) {
                       return (strcmp(layer, lp.layerName) == 0);
                     }) == kLayerProperties.end()) {
      THROW("Fail to find {}.", layer);
    }
    enabled_instance_layers.push_back(layer);
  }

  enabled_instance_extensions.reserve(required_instance_extensions.size());
  for (const char* extension : required_instance_extensions) {
    if (std::find_if(kExtensionProperties.begin(), kExtensionProperties.end(),
                     [extension](const vk::ExtensionProperties& ep) {
                       return (strcmp(extension, ep.extensionName) == 0);
                     }) == kExtensionProperties.end()) {
      THROW("Fail to find {}.", extension);
    }
    enabled_instance_extensions.push_back(extension);
  }

#ifndef NDEBUG
  if (std::find(
          required_instance_layers.begin(), required_instance_layers.end(),
          "VK_LAYER_KHRONOS_validation") == required_instance_layers.end() &&
      std::find_if(kLayerProperties.begin(), kLayerProperties.end(),
                   [](const vk::LayerProperties& lp) {
                     return (strcmp("VK_LAYER_KHRONOS_validation",
                                    lp.layerName) == 0);
                   }) != kLayerProperties.end()) {
    enabled_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
  }

  if (std::find(required_instance_extensions.begin(),
                required_instance_extensions.end(),
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME) ==
          required_instance_extensions.end() &&
      std::find_if(kExtensionProperties.begin(), kExtensionProperties.end(),
                   [](const vk::ExtensionProperties& ep) {
                     return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                    ep.extensionName) == 0);
                   }) != kExtensionProperties.end()) {
    enabled_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
#endif

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
      instance_ci{{flags, &application_info, enabled_instance_layers,
                   enabled_instance_extensions},
                  debug_utils_messenger_ci};
#else
  vk::StructureChain<vk::InstanceCreateInfo> instance_ci{
      {flags, &application_info, enabled_instance_layers,
       enabled_instance_extensions}};
#endif

  instance_ =
      vk::raii::Instance{context_, instance_ci.get<vk::InstanceCreateInfo>()};

#ifndef NDEBUG
  debug_utils_messenger_ = vk::raii::DebugUtilsMessengerEXT{
      instance_, instance_ci.get<vk::DebugUtilsMessengerCreateInfoEXT>()};
#endif
}

void Gpu::CreateSurface() {
  VkSurfaceKHR surface;
  window_->CreateWindowSurface(instance_, &surface);
  surface_ = vk::raii::SurfaceKHR{instance_, surface};
}

void Gpu::CreatePhysicalDevice() {
  vk::raii::PhysicalDevices physical_devices{instance_};

  u32 max_score{0};
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
  // Properties.
  vk::PhysicalDeviceProperties physical_device_properties_{
      physical_device_.getProperties()};

  // Queue famliy properties.
  std::vector<vk::QueueFamilyProperties> queue_family_properties{
      physical_device_.getQueueFamilyProperties()};
  u32 i{0};
  for (const auto& queue_famliy_propertie : queue_family_properties) {
    if ((queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eGraphics) &&
        (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eCompute) &&
        physical_device_.getSurfaceSupportKHR(i, *surface_)) {
      graphics_queue_index_ = i;
      compute_queue_index_ = i;
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
      if (physical_device_.getSurfaceSupportKHR(i, *surface_)) {
        present_queue_index_ = i;
      }
      if (graphics_queue_index_.has_value() &&
          compute_queue_index_.has_value() &&
          present_queue_index_.has_value()) {
        break;
      }
      ++i;
    }
  }

  if (!(graphics_queue_index_.has_value() && compute_queue_index_.has_value() &&
        present_queue_index_.has_value())) {
    THROW("Fail to find queue family.");
  }

  std::vector<vk::DeviceQueueCreateInfo> device_queue_cis;
  std::set<u32> queue_family_indexes{graphics_queue_index_.value(),
                                     compute_queue_index_.value(),
                                     present_queue_index_.value()};

  f32 queue_priority{0.0F};
  for (u32 queue_family_index : queue_family_indexes) {
    vk::DeviceQueueCreateInfo device_queue_ci{
        {}, queue_family_index, 1, &queue_priority};
    device_queue_cis.push_back(device_queue_ci);
  }

  // Extensions.
  std::vector<vk::ExtensionProperties> extension_properties{
      physical_device_.enumerateDeviceExtensionProperties()};

  std::vector<const char*> required_device_extensions_{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  std::vector<const char*> enabled_device_extensions;
  for (auto& extension : required_device_extensions_) {
    if (std::find_if(extension_properties.begin(), extension_properties.end(),
                     [extension](const vk::ExtensionProperties& ep) {
                       return (strcmp(extension, ep.extensionName) == 0);
                     }) == extension_properties.end()) {
      THROW("Don't find {}", extension);
    }
    enabled_device_extensions.push_back(extension);
  }

#ifdef __APPLE__
  enabled_device_extensions.push_back("VK_KHR_portability_subset");
#endif

  // Features.
  vk::PhysicalDeviceFeatures physical_device_features;

  vk::PhysicalDeviceDescriptorIndexingFeatures indexing_features;
  indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
  indexing_features.runtimeDescriptorArray = VK_TRUE;
  indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

  vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features;
  dynamic_rendering_features.dynamicRendering = VK_TRUE;

  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceDescriptorIndexingFeatures,
                     vk::PhysicalDeviceDynamicRenderingFeatures>
      physical_device_features2{physical_device_features, indexing_features,
                                dynamic_rendering_features};

  // Create device.
  vk::DeviceCreateInfo device_ci{{},      device_queue_cis,
                                 {},      enabled_device_extensions,
                                 nullptr, &physical_device_features2};

  device_ = vk::raii::Device{physical_device_, device_ci};

  // Create queue.
  graphics_queue_ = vk::raii::Queue{device_, graphics_queue_index_.value(), 0};
  compute_queue_ = vk::raii::Queue{device_, compute_queue_index_.value(), 0};
  present_queue_ = vk::raii::Queue{device_, present_queue_index_.value(), 0};
}

void Gpu::CreateAllocator() {
  VmaAllocatorCreateInfo allocator_ci{
      .flags = 0,
      .physicalDevice = static_cast<VkPhysicalDevice>(*physical_device_),
      .device = static_cast<VkDevice>(*device_),
      .instance = static_cast<VkInstance>(*instance_),
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };
  vmaCreateAllocator(&allocator_ci, &allocator_);
}

void Gpu::CreateCommandObjects() {
  vk::CommandPoolCreateInfo command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      graphics_queue_index_.value()};

  command_pool_ = vk::raii::CommandPool{device_, command_pool_ci};

  vk::CommandBufferAllocateInfo command_buffer_allocate_info{
      *command_pool_, vk::CommandBufferLevel::ePrimary, kCommandBufferCount};

  command_buffers_ =
      vk::raii::CommandBuffers{device_, command_buffer_allocate_info};
}

void Gpu::CreateDescriptorPool() {
  std::vector<vk::DescriptorPoolSize> pool_sizes{
      {vk::DescriptorType::eSampler, 1000},
      {vk::DescriptorType::eCombinedImageSampler, 1000},
      {vk::DescriptorType::eSampledImage, 1000},
      {vk::DescriptorType::eStorageImage, 1000},
      {vk::DescriptorType::eUniformTexelBuffer, 1000},
      {vk::DescriptorType::eStorageTexelBuffer, 1000},
      {vk::DescriptorType::eUniformBuffer, 1000},
      {vk::DescriptorType::eStorageBuffer, 1000},
      {vk::DescriptorType::eUniformBufferDynamic, 1000},
      {vk::DescriptorType::eStorageBufferDynamic, 1000},

      {vk::DescriptorType::eInputAttachment, 1000}};

  u32 max_sets{std::accumulate(pool_sizes.begin(), pool_sizes.end(),
                               static_cast<u32>(0),
                               [](u32 sum, const vk::DescriptorPoolSize& dps) {
                                 return sum + dps.descriptorCount;
                               })};

  vk::DescriptorPoolCreateInfo descriptor_pool_ci{
      vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, max_sets,
      pool_sizes};
  descriptor_pool_ = vk::raii::DescriptorPool{device_, descriptor_pool_ci};
}

void Gpu::SetObjectName(vk::ObjectType object_type, u64 handle,
                        const std::string& name, const std::string& suffix) {
  if (!name.empty()) {
    std::string name_suffix{suffix + " " + name};
    vk::DebugUtilsObjectNameInfoEXT buffer_name_info{object_type, handle,
                                                     name_suffix.c_str()};
    device_.setDebugUtilsObjectNameEXT(buffer_name_info);
  }
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
    for (u32 i{0}; i < p_callback_data->queueLabelCount; i++) {
      LOGI("\t\tLabel name = [{}]",
           p_callback_data->pQueueLabels[i].pLabelName);
    }
  }
  if (p_callback_data->cmdBufLabelCount > 0) {
    LOGI("\tCommand buffer labels:");
    for (u32 i{0}; i < p_callback_data->cmdBufLabelCount; i++) {
      LOGI("\t\tLabel name = [{}]",
           p_callback_data->pCmdBufLabels[i].pLabelName);
    }
  }
  if (p_callback_data->objectCount > 0) {
    LOGI("\tObjects:");
    for (u32 i{0}; i < p_callback_data->objectCount; i++) {
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
