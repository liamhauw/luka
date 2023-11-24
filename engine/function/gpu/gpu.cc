// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"

#include "context.h"
#include "core/log.h"
#include "function/window/window.h"

namespace luka {

Gpu::Gpu() {
  CreateInstance();
  CreateSurface();
  CreatePhysicalDevice();
  CreateDevice();
  CreateQueryPool();
  CreateSwapchain();
  CreateRenderPass();
  CreateFramebuffers();
  CreateCommandObjects();
  CreateSyncObjects();
  CreatePipelineCache();
  CreateDescriptorPool();
  CreateResourceManager();
}

Gpu::~Gpu() { device_.waitIdle(); }

void Gpu::Tick() {
  if (gContext.window->GetFramebufferResized()) {
    gContext.window->SetFramebufferResized(false);
    Resize();
  }
}

void Gpu::Resize() {
  CreateSwapchain();
  CreateFramebuffers();
}

void Gpu::BeginFrame() { used_command_buffer_counts_[back_buffer_index] = 0; }

void Gpu::EndFrame() {
  back_buffer_index = (back_buffer_index + 1) % kBackBufferCount;
}

const vk::raii::CommandBuffer& Gpu::GetCommandBuffer() {
  u32 index{used_command_buffer_counts_[back_buffer_index]};
  if (index >= kMaxUsedCommandBufferCountperFrame) {
    THROW("Fail to get command buffer.");
  }
  ++used_command_buffer_counts_[back_buffer_index];

  return command_buffers_[back_buffer_index][index];
}

ImGui_ImplVulkan_InitInfo Gpu::GetVulkanInitInfoForImgui() {
  ImGui_ImplVulkan_InitInfo init_info{
      .Instance = static_cast<VkInstance>(*instance_),
      .PhysicalDevice = static_cast<VkPhysicalDevice>(*physical_device_),
      .Device = static_cast<VkDevice>(*device_),
      .QueueFamily = graphics_queue_index_.value(),
      .Queue = static_cast<VkQueue>(*graphics_queue_),
      .PipelineCache = static_cast<VkPipelineCache>(*pipeline_cache_),
      .DescriptorPool = static_cast<VkDescriptorPool>(*descriptor_pool_),
      .Subpass = 0,
      .MinImageCount = image_count_,
      .ImageCount = image_count_,
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
  };
  return init_info;
}

VkRenderPass Gpu::GetRenderPass() {
  return static_cast<VkRenderPass>(*render_pass_);
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
      gContext.window->GetRequiredInstanceExtensions()};
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
      info_chain{{flags, &application_info, enabled_instance_layers,
                  enabled_instance_extensions},
                 debug_utils_messenger_ci};
#else
  vk::StructureChain<vk::InstanceCreateInfo> info_chain{
      {flags, &application_info, enabled_instance_layers,
       enabled_instance_extensions}};
#endif

  instance_ =
      vk::raii::Instance{context_, info_chain.get<vk::InstanceCreateInfo>()};

#ifndef NDEBUG
  debug_utils_messenger_ = vk::raii::DebugUtilsMessengerEXT{
      instance_, info_chain.get<vk::DebugUtilsMessengerCreateInfoEXT>()};
#endif
}

void Gpu::CreateSurface() {
  VkSurfaceKHR surface;
  gContext.window->CreateWindowSurface(instance_, &surface);
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

  vk::SampleCountFlags sample_count{
      physical_device_properties_.limits.framebufferColorSampleCounts &
      physical_device_properties_.limits.framebufferDepthSampleCounts};
  if (sample_count & vk::SampleCountFlagBits::e64) {
    sample_count_ = vk::SampleCountFlagBits::e64;
  } else if (sample_count & vk::SampleCountFlagBits::e32) {
    sample_count_ = vk::SampleCountFlagBits::e32;
  } else if (sample_count & vk::SampleCountFlagBits::e16) {
    sample_count_ = vk::SampleCountFlagBits::e16;
  } else if (sample_count & vk::SampleCountFlagBits::e8) {
    sample_count_ = vk::SampleCountFlagBits::e8;
  } else if (sample_count & vk::SampleCountFlagBits::e4) {
    sample_count_ = vk::SampleCountFlagBits::e4;
  } else if (sample_count & vk::SampleCountFlagBits::e2) {
    sample_count_ = vk::SampleCountFlagBits::e2;
  } else {
    sample_count_ = vk::SampleCountFlagBits::e1;
  }

  max_anisotropy_ = physical_device_properties_.limits.maxSamplerAnisotropy;

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

  f32 queue_priority{0.0f};
  for (u32 queue_family_index : queue_family_indexes) {
    vk::DeviceQueueCreateInfo device_queue_ci{
        {}, queue_family_index, 1, &queue_priority};
    device_queue_cis.push_back(device_queue_ci);
  }

  // Extensions.
  std::vector<vk::ExtensionProperties> extension_properties{
      physical_device_.enumerateDeviceExtensionProperties()};

  std::vector<const char*> required_device_extensions_{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_ROBUSTNESS_2_EXTENSION_NAME};

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
  physical_device_features.independentBlend = VK_TRUE;
  physical_device_features.fillModeNonSolid = VK_TRUE;
  physical_device_features.wideLines = VK_TRUE;
  physical_device_features.samplerAnisotropy = VK_TRUE;
  physical_device_features.pipelineStatisticsQuery = VK_TRUE;
  physical_device_features.vertexPipelineStoresAndAtomics = VK_TRUE;
  physical_device_features.shaderImageGatherExtended = VK_TRUE;

  vk::PhysicalDeviceVulkan11Features physical_device_vulkan11_features;
  physical_device_vulkan11_features.storageBuffer16BitAccess = VK_TRUE;

  vk::PhysicalDeviceVulkan12Features physical_device_vulkan12_features;
  physical_device_vulkan12_features.shaderFloat16 = VK_TRUE;
  physical_device_vulkan12_features.shaderSubgroupExtendedTypes = VK_TRUE;

  vk::PhysicalDeviceRobustness2FeaturesEXT robustness2;
  robustness2.nullDescriptor = VK_TRUE;

  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan12Features,
                     vk::PhysicalDeviceRobustness2FeaturesEXT>
      physical_device_features2{physical_device_features,
                                physical_device_vulkan11_features,
                                physical_device_vulkan12_features, robustness2};

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

void Gpu::CreateQueryPool() {
  vk::QueryPoolCreateInfo query_pool_ci{
      {}, vk::QueryType::eTimestamp, 128 * kBackBufferCount};

  query_pool_ = vk::raii::QueryPool{device_, query_pool_ci};
}

void Gpu::CreateSwapchain() {
  // Image count.
  vk::SurfaceCapabilitiesKHR surface_capabilities{
      physical_device_.getSurfaceCapabilitiesKHR(*surface_)};
  image_count_ = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      image_count_ > surface_capabilities.maxImageCount) {
    image_count_ = surface_capabilities.maxImageCount;
  }

  // Format and color space.
  std::vector<vk::SurfaceFormatKHR> surface_formats{
      physical_device_.getSurfaceFormatsKHR(*(surface_))};

  vk::SurfaceFormatKHR picked_format{surface_formats[0]};

  std::vector<vk::Format> requested_formats{
      vk::Format::eB8G8R8A8Srgb, vk::Format::eR8G8B8A8Srgb,
      vk::Format::eB8G8R8Srgb, vk::Format::eR8G8B8Srgb};
  vk::ColorSpaceKHR requested_color_space{vk::ColorSpaceKHR::eSrgbNonlinear};
  for (const auto& requested_format : requested_formats) {
    auto it{std::find_if(surface_formats.begin(), surface_formats.end(),
                         [requested_format, requested_color_space](
                             const vk::SurfaceFormatKHR& f) {
                           return (f.format == requested_format) &&
                                  (f.colorSpace == requested_color_space);
                         })};
    if (it != surface_formats.end()) {
      picked_format = *it;
      break;
    }
  }

  format_ = picked_format.format;
  color_space_ = picked_format.colorSpace;

  // Extent.
  if (surface_capabilities.currentExtent.width ==
      std::numeric_limits<u32>::max()) {
    i32 width{0};
    i32 height{0};
    gContext.window->GetFramebufferSize(&width, &height);

    extent_.width = std::clamp(static_cast<u32>(width),
                               surface_capabilities.minImageExtent.width,
                               surface_capabilities.maxImageExtent.width);
    extent_.height = std::clamp(static_cast<u32>(height),
                                surface_capabilities.minImageExtent.height,
                                surface_capabilities.maxImageExtent.height);
  } else {
    extent_ = surface_capabilities.currentExtent;
  }

  // Present mode.
  std::vector<vk::PresentModeKHR> present_modes{
      physical_device_.getSurfacePresentModesKHR(*surface_)};

  vk::PresentModeKHR picked_mode{vk::PresentModeKHR::eFifo};
  for (const auto& present_mode : present_modes) {
    if (present_mode == vk::PresentModeKHR::eMailbox) {
      picked_mode = present_mode;
      break;
    }

    if (present_mode == vk::PresentModeKHR::eImmediate) {
      picked_mode = present_mode;
    }
  }
  present_mode_ = picked_mode;

  // Create swapchain
  vk::SwapchainCreateInfoKHR swapchain_ci{
      {},
      *surface_,
      image_count_,
      format_,
      color_space_,
      extent_,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {},
      surface_capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      present_mode_,
      VK_TRUE,
      {}};

  if (graphics_queue_index_.value() != present_queue_index_.value()) {
    u32 queue_family_indices[2]{graphics_queue_index_.value(),
                                present_queue_index_.value()};
    swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queue_family_indices;
  }

  swapchain_.clear();
  swapchain_ = vk::raii::SwapchainKHR{device_, swapchain_ci};

  // Get swapchain images.
  images_ = swapchain_.getImages();

  // Create swapchain image views
  image_views_.clear();
  image_views_.reserve(images_.size());
  vk::ImageViewCreateInfo image_view_ci{
      {},      {}, vk::ImageViewType::e2D,
      format_, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  for (auto image : images_) {
    image_view_ci.image = image;
    image_views_.emplace_back(device_, image_view_ci);
  }
}

void Gpu::CreateRenderPass() {
  std::vector<vk::AttachmentDescription> attachment_descriptions;

  attachment_descriptions.emplace_back(
      vk::AttachmentDescriptionFlags{}, format_, vk::SampleCountFlagBits::e1,
      vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
      vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
      vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference color_attachment_ref{
      0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpass_description{
      {}, vk::PipelineBindPoint::eGraphics, {}, color_attachment_ref};

  vk::SubpassDependency subpass_dependency{
      VK_SUBPASS_EXTERNAL,
      0,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      {},
      vk::AccessFlagBits::eColorAttachmentRead |
          vk::AccessFlagBits::eColorAttachmentWrite};

  vk::RenderPassCreateInfo render_pass_ci{
      {}, attachment_descriptions, subpass_description, subpass_dependency};

  render_pass_ = vk::raii::RenderPass{device_, render_pass_ci};
}

void Gpu::CreateFramebuffers() {
  std::vector<vk::ImageView> attachements(1);
  vk::FramebufferCreateInfo framebuffer_ci{
      {}, *render_pass_, attachements, extent_.width, extent_.height, 1};

  framebuffers_.clear();
  framebuffers_.reserve(image_views_.size());
  for (const auto& image_view : image_views_) {
    attachements[0] = *image_view;
    framebuffers_.emplace_back(device_, framebuffer_ci);
  }
}

void Gpu::CreateCommandObjects() {
  used_command_buffer_counts_ = std::vector<u32>(kBackBufferCount, 0);
  command_pools_.reserve(kBackBufferCount);
  command_buffers_.reserve(kBackBufferCount);

  for (u32 i{0}; i < kBackBufferCount; ++i) {
    command_pools_.emplace_back(vk::raii::CommandPool{
        device_,
        {vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
         graphics_queue_index_.value()}});

    vk::CommandBufferAllocateInfo command_buffer_allocate_info{
        *command_pools_[i], vk::CommandBufferLevel::ePrimary,
        kMaxUsedCommandBufferCountperFrame};
    command_buffers_.emplace_back(
        vk::raii::CommandBuffers{device_, command_buffer_allocate_info});
  }
}

void Gpu::CreateSyncObjects() {
  command_executed_fences_.reserve(kBackBufferCount);
  image_available_semaphores_.reserve(kBackBufferCount);
  render_finished_semaphores_.reserve(kBackBufferCount);

  vk::FenceCreateInfo fence_ci{vk::FenceCreateFlagBits::eSignaled};
  vk::SemaphoreCreateInfo semaphore_ci{};

  for (u32 i{0}; i < kBackBufferCount; ++i) {
    command_executed_fences_.emplace_back(device_, fence_ci);
    image_available_semaphores_.emplace_back(device_, semaphore_ci);
    render_finished_semaphores_.emplace_back(device_, semaphore_ci);
  }
}

void Gpu::CreatePipelineCache() {
  vk::PipelineCacheCreateInfo pipeline_cache_ci;
  pipeline_cache_ = vk::raii::PipelineCache{device_, pipeline_cache_ci};
}

void Gpu::CreateDescriptorPool() {
  std::vector<vk::DescriptorPoolSize> pool_sizes{
      {vk::DescriptorType::eSampler, 1000},
      {vk::DescriptorType::eCombinedImageSampler, 1000},
      {vk::DescriptorType::eSampledImage, 1000},
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

void Gpu::CreateResourceManager() {
  resource_manager_ = std::make_unique<VmaResourceManager>(
      instance_, physical_device_, device_);
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
