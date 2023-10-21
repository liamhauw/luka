/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw

  Instance source file.
*/

#include "function/rendering/gpu.h"

#include <set>

#include "core/log.h"
#include "function/window/window.h"

namespace luka {

Gpu::Gpu(std::shared_ptr<Window> window) : window_{window} {
  CreateBase();
  CreateDevice();
  CreateSwapchain();
}

Gpu::~Gpu() { device_.waitIdle(); }

void Gpu::Resize() { CreateSwapchain(); }

void Gpu::CreateBase() {
  vk::InstanceCreateFlags flags;
#ifdef __APPLE__
  flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

  vk::ApplicationInfo application_info{"luka", VK_MAKE_VERSION(1, 0, 0), "luka",
                                       VK_MAKE_VERSION(1, 0, 0),
                                       VK_API_VERSION_1_2};

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

  std::vector<const char*> enabled_layers;
  std::vector<const char*> enabled_extensions;

  enabled_layers.reserve(required_instance_layers.size());
  for (const char* layer : required_instance_layers) {
    if (std::find_if(kLayerProperties.begin(), kLayerProperties.end(),
                     [layer](const vk::LayerProperties& lp) {
                       return (strcmp(layer, lp.layerName) == 0);
                     }) == kLayerProperties.end()) {
      THROW("Fail to find {}.", layer);
    }
    enabled_layers.push_back(layer);
  }

  enabled_extensions.reserve(required_instance_extensions.size());
  for (const char* extension : required_instance_extensions) {
    if (std::find_if(kExtensionProperties.begin(), kExtensionProperties.end(),
                     [extension](const vk::ExtensionProperties& ep) {
                       return (strcmp(extension, ep.extensionName) == 0);
                     }) == kExtensionProperties.end()) {
      THROW("Fail to find {}.", extension);
    }
    enabled_extensions.push_back(extension);
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
    enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
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
    enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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

  vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{
      {},
      message_severity_flags,
      message_type_flags,
      &DebugUtilsMessengerCallback};

  vk::StructureChain<vk::InstanceCreateInfo,
                     vk::DebugUtilsMessengerCreateInfoEXT>
      info_chain{{flags, &application_info, enabled_layers, enabled_extensions},
                 debug_utils_messenger_create_info};
#else
  vk::StructureChain<vk::InstanceCreateInfo> info_chain{
      {flags, &application_info, enabled_layers, enabled_extensions}};
#endif

  instance_ =
      vk::raii::Instance{context_, info_chain.get<vk::InstanceCreateInfo>()};

#ifndef NDEBUG
  debug_utils_messenger_ = vk::raii::DebugUtilsMessengerEXT{
      instance_, info_chain.get<vk::DebugUtilsMessengerCreateInfoEXT>()};
#endif

  VkSurfaceKHR surface;
  window_->CreateWindowSurface(instance_, &surface);
  surface_ = vk::raii::SurfaceKHR{instance_, surface};

  vk::raii::PhysicalDevices physical_devices{instance_};

  uint32_t max_score{0};
  for (auto& physical_device : physical_devices) {
    uint32_t cur_score{1};

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

  // Queue famliy.
  std::vector<vk::QueueFamilyProperties> queue_family_properties{
      physical_device_.getQueueFamilyProperties()};
  uint32_t i{0};
  for (const auto& queue_famliy_propertie : queue_family_properties) {
    if ((queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eGraphics) &&
        (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eCompute) &&
        physical_device_.getSurfaceSupportKHR(i, *surface_)) {
      queue_family_.graphics_index = i;
      queue_family_.compute_index = i;
      queue_family_.present_index = i;
      break;
    }
    ++i;
  }
  if (!queue_family_.IsComplete()) {
    i = 0;
    for (const auto& queue_famliy_propertie : queue_family_properties) {
      if (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eGraphics) {
        queue_family_.graphics_index = i;
      }
      if (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eCompute) {
        queue_family_.compute_index = i;
      }
      if (physical_device_.getSurfaceSupportKHR(i, *surface_)) {
        queue_family_.present_index = i;
      }
      if (queue_family_.IsComplete()) {
        break;
      }
      ++i;
    }
  }

  if (!queue_family_.IsComplete()) {
    THROW("Fail to find queue family.");
  }

  std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos;
  std::set<uint32_t> queue_family_indexes{queue_family_.graphics_index.value(),
                                          queue_family_.compute_index.value(),
                                          queue_family_.present_index.value()};

  float queue_priority{0.0f};
  for (uint32_t queue_family_index : queue_family_indexes) {
    vk::DeviceQueueCreateInfo device_queue_create_info{
        {}, queue_family_index, 1, &queue_priority};
    device_queue_create_infos.push_back(device_queue_create_info);
  }

  // Extensions.
  std::vector<const char*> enabled_extensions;
  std::vector<vk::ExtensionProperties> extension_properties{
      physical_device_.enumerateDeviceExtensionProperties()};
  for (auto& extension : device_extensions_) {
    auto& extension_name{extension.first};
    auto& extension_enable{extension.second};
    if (!extension_enable) {
      continue;
    }
    if (std::find_if(extension_properties.begin(), extension_properties.end(),
                     [extension_name](const vk::ExtensionProperties& ep) {
                       return (strcmp(extension_name, ep.extensionName) == 0);
                     }) == extension_properties.end()) {
      LOGI("Don't find {}, disable it.", extension_name);
      extension_enable = false;
      continue;
    }
    enabled_extensions.push_back(extension_name);
  }

#ifdef __APPLE__
  enabled_extensions.push_back("VK_KHR_portability_subset");
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

  vk::PhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate;
  if (device_extensions_[VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME]) {
    fragment_shading_rate.pipelineFragmentShadingRate = VK_TRUE;
    fragment_shading_rate.primitiveFragmentShadingRate = VK_TRUE;
    fragment_shading_rate.attachmentFragmentShadingRate = VK_TRUE;
  }

  vk::PhysicalDeviceRobustness2FeaturesEXT robustness2;
  if (device_extensions_[VK_EXT_ROBUSTNESS_2_EXTENSION_NAME]) {
    robustness2.nullDescriptor = VK_TRUE;
  }

  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan12Features,
                     vk::PhysicalDeviceFragmentShadingRateFeaturesKHR,
                     vk::PhysicalDeviceRobustness2FeaturesEXT>
      physical_device_features2{physical_device_features,
                                physical_device_vulkan11_features,
                                physical_device_vulkan12_features,
                                fragment_shading_rate, robustness2};

  // Create device.
  vk::DeviceCreateInfo device_create_info{{},      device_queue_create_infos,
                                          {},      enabled_extensions,
                                          nullptr, &physical_device_features2};

  device_ = vk::raii::Device{physical_device_, device_create_info};

  // Create queue.
  graphics_queue_ =
      vk::raii::Queue{device_, queue_family_.graphics_index.value(), 0};
  compute_queue_ =
      vk::raii::Queue{device_, queue_family_.compute_index.value(), 0};
  present_queue_ =
      vk::raii::Queue{device_, queue_family_.present_index.value(), 0};
}

void Gpu::CreateSwapchain() {
  // Image count.
  vk::SurfaceCapabilitiesKHR surface_capabilities{
      physical_device_.getSurfaceCapabilitiesKHR(*surface_)};
  swapchain_data_.image_count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      swapchain_data_.image_count > surface_capabilities.maxImageCount) {
    swapchain_data_.image_count = surface_capabilities.maxImageCount;
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

  swapchain_data_.format = picked_format.format;
  swapchain_data_.color_space = picked_format.colorSpace;

  // Extent.
  if (surface_capabilities.currentExtent.width ==
      std::numeric_limits<uint32_t>::max()) {
    int width{0};
    int height{0};
    window_->GetFramebufferSize(&width, &height);

    swapchain_data_.extent.width = std::clamp(
        static_cast<uint32_t>(width), surface_capabilities.minImageExtent.width,
        surface_capabilities.maxImageExtent.width);
    swapchain_data_.extent.height =
        std::clamp(static_cast<uint32_t>(height),
                   surface_capabilities.minImageExtent.height,
                   surface_capabilities.maxImageExtent.height);
  } else {
    swapchain_data_.extent = surface_capabilities.currentExtent;
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
  swapchain_data_.present_mode = picked_mode;

  // Create swapchain
  vk::SwapchainCreateInfoKHR swapchain_create_info{
      {},
      *surface_,
      swapchain_data_.image_count,
      swapchain_data_.format,
      swapchain_data_.color_space,
      swapchain_data_.extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {},
      surface_capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      swapchain_data_.present_mode,
      VK_TRUE,
      {}};

  if (queue_family_.graphics_index.value() !=
      queue_family_.present_index.value()) {
    uint32_t queue_family_indices[2]{queue_family_.graphics_index.value(),
                                     queue_family_.present_index.value()};
    swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
    swapchain_create_info.queueFamilyIndexCount = 2;
    swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
  }

  swapchain_data_.swapchain.clear();
  swapchain_data_.swapchain =
      vk::raii::SwapchainKHR{device_, swapchain_create_info};

  // Get swapchain images.
  swapchain_data_.images = swapchain_data_.swapchain.getImages();

  // Create swapchain image views
  swapchain_data_.image_views.clear();
  swapchain_data_.image_views.reserve(swapchain_data_.images.size());
  vk::ImageViewCreateInfo image_view_create_info{
      {},
      {},
      vk::ImageViewType::e2D,
      swapchain_data_.format,
      {},
      {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  for (auto image : swapchain_data_.images) {
    image_view_create_info.image = image;
    swapchain_data_.image_views.emplace_back(device_, image_view_create_info);
  }

  // Create sync objects
  command_executed_fences_.reserve(swapchain_data_.image_count);
  image_available_semaphores_.reserve(swapchain_data_.image_count);
  render_finished_semaphores_.reserve(swapchain_data_.image_count);

  vk::FenceCreateInfo fence_create_info{vk::FenceCreateFlagBits::eSignaled};
  vk::SemaphoreCreateInfo semaphore_create_info{};

  for (uint32_t i{0}; i < swapchain_data_.image_count; ++i) {
    command_executed_fences_.emplace_back(device_, fence_create_info);
    image_available_semaphores_.emplace_back(device_, semaphore_create_info);
    render_finished_semaphores_.emplace_back(device_, semaphore_create_info);
  }

  // Create render pass.
  std::vector<vk::AttachmentDescription> attachment_descriptions;

  attachment_descriptions.emplace_back(
      vk::AttachmentDescriptionFlags{}, swapchain_data_.format,
      vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
      vk::ImageLayout::ePresentSrcKHR);

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

  vk::RenderPassCreateInfo render_pass_create_info{
      {}, attachment_descriptions, subpass_description, subpass_dependency};

  render_pass_ = vk::raii::RenderPass{device_, render_pass_create_info};

  // Create framebuffer
  std::vector<vk::ImageView> attachements(1);
  vk::FramebufferCreateInfo framebuffer_create_info{
      {},
      *render_pass_,
      attachements,
      swapchain_data_.extent.width,
      swapchain_data_.extent.height,
      1};

  framebuffers_.clear();
  framebuffers_.reserve(swapchain_data_.image_views.size());
  for (const auto& image_view : swapchain_data_.image_views) {
    attachements[0] = *image_view;
    framebuffers_.emplace_back(device_, framebuffer_create_info);
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
    for (uint32_t i{0}; i < p_callback_data->queueLabelCount; i++) {
      LOGI("\t\tLabel name = [{}]",
           p_callback_data->pQueueLabels[i].pLabelName);
    }
  }
  if (p_callback_data->cmdBufLabelCount > 0) {
    LOGI("\tCommand buffer labels:");
    for (uint32_t i{0}; i < p_callback_data->cmdBufLabelCount; i++) {
      LOGI("\t\tLabel name = [{}]",
           p_callback_data->pCmdBufLabels[i].pLabelName);
    }
  }
  if (p_callback_data->objectCount > 0) {
    LOGI("\tObjects:");
    for (uint32_t i{0}; i < p_callback_data->objectCount; i++) {
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
