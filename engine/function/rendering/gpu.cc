/*
  SPDX license identifier: MIT.

  Copyright (C) 2023-present Liam Hauw.

  GPU source file.
*/

#include "function/rendering/gpu.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "context.h"
#include "core/log.h"

namespace luka {

Gpu::Gpu() {
  MakeInstance();
  MakeSurface();
  MakePhysicalDevice();
  MakeDevice();
  MakeSwapchain();
  MakeCommandObjects();
  MakeSyncObjects();
  MakeDepthImage();
}

Gpu::~Gpu() { device_.waitIdle(); }

void Gpu::MakeInstance() {
  vk::ApplicationInfo application_info{"luka", VK_MAKE_VERSION(1, 0, 0), "luka",
                                       VK_MAKE_VERSION(1, 0, 0),
                                       VK_API_VERSION_1_0};

  std::vector<const char*> window_required_instance_extensions{
      gContext.window->GetRequiredInstanceExtension()};
  required_instance_extensions_.insert(
      required_instance_extensions_.end(),
      window_required_instance_extensions.begin(),
      window_required_instance_extensions.end());

#ifdef __APPLE__
  required_instance_extensions_.push_back(
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  required_instance_extensions_.push_back(
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

  const std::vector<vk::LayerProperties> kLayerProperties{
      context_.enumerateInstanceLayerProperties()};
  const std::vector<vk::ExtensionProperties> kExtensionProperties{
      context_.enumerateInstanceExtensionProperties()};

  std::vector<const char*> enabled_layers;
  std::vector<const char*> enabled_extensions;

  enabled_layers.reserve(required_instance_layers_.size());
  for (const char* layer : required_instance_layers_) {
    if (std::find_if(kLayerProperties.begin(), kLayerProperties.end(),
                     [layer](const vk::LayerProperties& lp) {
                       return (strcmp(layer, lp.layerName) == 0);
                     }) == kLayerProperties.end()) {
      THROW("Fail to find {}.", layer);
    }
    enabled_layers.push_back(layer);
  }

  enabled_extensions.reserve(required_instance_extensions_.size());
  for (const char* extension : required_instance_extensions_) {
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
          required_instance_layers_.begin(), required_instance_layers_.end(),
          "VK_LAYER_KHRONOS_validation") == required_instance_layers_.end() &&
      std::find_if(kLayerProperties.begin(), kLayerProperties.end(),
                   [](const vk::LayerProperties& lp) {
                     return (strcmp("VK_LAYER_KHRONOS_validation",
                                    lp.layerName) == 0);
                   }) != kLayerProperties.end()) {
    enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
  }

  if (std::find(required_instance_extensions_.begin(),
                required_instance_extensions_.end(),
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME) ==
          required_instance_extensions_.end() &&
      std::find_if(kExtensionProperties.begin(), kExtensionProperties.end(),
                   [](const vk::ExtensionProperties& ep) {
                     return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                    ep.extensionName) == 0);
                   }) != kExtensionProperties.end()) {
    enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
#endif

  vk::InstanceCreateFlags flags;
#ifdef __APPLE__
  flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
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
      instance_, debug_utils_messenger_create_info};
#endif
}

void Gpu::MakeSurface() {
  VkSurfaceKHR surface;
  gContext.window->CreateWindowSurface(instance_, &surface);

  surface_ = vk::raii::SurfaceKHR{instance_, surface};
}

void Gpu::MakePhysicalDevice() {
  vk::raii::PhysicalDevices physical_devices{instance_};

  uint32_t max_score{0};
  for (auto& physical_device : physical_devices) {
    uint32_t cur_score{1};

    // Queue famliy.
    QueueFamliy queue_family;

    std::vector<vk::QueueFamilyProperties> queue_family_properties{
        physical_device.getQueueFamilyProperties()};
    uint32_t i{0};
    for (const auto& queue_famliy_propertie : queue_family_properties) {
      if ((queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eGraphics) &&
          (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eCompute) &&
          physical_device.getSurfaceSupportKHR(i, *surface_)) {
        queue_family.graphics_index = i;
        queue_family.compute_index = i;
        queue_family.present_index = i;
        break;
      }
      ++i;
    }
    if (!queue_family.IsComplete()) {
      i = 0;
      for (const auto& queue_famliy_propertie : queue_family_properties) {
        if (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eGraphics) {
          queue_family.graphics_index = i;
        }
        if (queue_famliy_propertie.queueFlags & vk::QueueFlagBits::eCompute) {
          queue_family.compute_index = i;
        }
        if (physical_device.getSurfaceSupportKHR(i, *surface_)) {
          queue_family.present_index = i;
        }
        if (queue_family.IsComplete()) {
          break;
        }
        ++i;
      }
    }

    // Device extension.
    bool has_device_extension{true};
    std::vector<vk::ExtensionProperties> extension_properties{
        physical_device.enumerateDeviceExtensionProperties()};
    for (const char* device_extension : required_device_extensions_) {
      if (std::find_if(extension_properties.begin(), extension_properties.end(),
                       [device_extension](const vk::ExtensionProperties& ep) {
                         return (strcmp(device_extension, ep.extensionName) ==
                                 0);
                       }) == extension_properties.end()) {
        has_device_extension = false;
        break;
      }
    }

    // Device feature.
    vk::PhysicalDeviceFeatures physical_device_features{
        physical_device.getFeatures()};
    bool has_device_feature{
        static_cast<bool>(physical_device_features.samplerAnisotropy &
                          physical_device_features.fragmentStoresAndAtomics &
                          physical_device_features.independentBlend)};

    // Requirements not met.
    if (!queue_family.IsComplete() || !has_device_extension ||
        !has_device_feature) {
      continue;
    }

    // The gpu with the highest score is selected.
    vk::PhysicalDeviceProperties physical_device_properties{
        physical_device.getProperties()};
    if (physical_device_properties.deviceType ==
        vk::PhysicalDeviceType::eDiscreteGpu) {
      cur_score += 100;
    }
    if (cur_score > max_score) {
      max_score = cur_score;
      physical_device_ = std::move(physical_device);
      queue_family_ = queue_family;
    }
  }

  if (!(*physical_device_)) {
    THROW("Fail to find physical device.");
  }

  vk::PhysicalDeviceProperties physical_device_properties{
      physical_device_.getProperties()};

  vk::SampleCountFlags sample_count{
      physical_device_properties.limits.framebufferColorSampleCounts &
      physical_device_properties.limits.framebufferDepthSampleCounts};
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

  max_anisotropy_ = physical_device_properties.limits.maxSamplerAnisotropy;
}

void Gpu::MakeDevice() {
  std::vector<const char*> enabled_extensions{required_device_extensions_};
#ifdef __APPLE__
  enabled_extensions.push_back("VK_KHR_portability_subset");
#endif

  std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos;
  std::set<uint32_t> queue_family_indexes{queue_family_.graphics_index.value(),
                                          queue_family_.compute_index.value(),
                                          queue_family_.present_index.value()};

  float queue_priority{1.0f};
  for (uint32_t queue_family_index : queue_family_indexes) {
    vk::DeviceQueueCreateInfo device_queue_create_info{
        {}, queue_family_index, 1, &queue_priority};
    device_queue_create_infos.push_back(device_queue_create_info);
  }

  vk::PhysicalDeviceFeatures physical_device_features{};
  physical_device_features.samplerAnisotropy = VK_TRUE;
  physical_device_features.fragmentStoresAndAtomics = VK_TRUE;
  physical_device_features.independentBlend = VK_TRUE;

  vk::DeviceCreateInfo device_create_info{{},
                                          device_queue_create_infos,
                                          {},
                                          enabled_extensions,
                                          &physical_device_features};

  device_ = vk::raii::Device{physical_device_, device_create_info};

  graphics_queue_ =
      vk::raii::Queue{device_, queue_family_.graphics_index.value(), 0};
  compute_queue_ =
      vk::raii::Queue{device_, queue_family_.compute_index.value(), 0};
  present_queue_ =
      vk::raii::Queue{device_, queue_family_.present_index.value(), 0};
}

void Gpu::MakeSwapchain() {
  vk::SurfaceCapabilitiesKHR surface_capabilities{
      physical_device_.getSurfaceCapabilitiesKHR(*surface_)};

  // Image count.
  swapchain_data_.count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      swapchain_data_.count > surface_capabilities.maxImageCount) {
    swapchain_data_.count = surface_capabilities.maxImageCount;
  }

  // Extent.
  if (surface_capabilities.currentExtent.width ==
      std::numeric_limits<uint32_t>::max()) {
    int width, height;
    gContext.window->GetFramebufferSize(&width, &height);

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

  // Format.
  std::vector<vk::SurfaceFormatKHR> surface_formats{
      physical_device_.getSurfaceFormatsKHR(*(surface_))};

  vk::SurfaceFormatKHR picked_format{surface_formats[0]};
  if (surface_formats.size() == 1) {
    if (surface_formats[0].format == vk::Format::eUndefined) {
      picked_format.format = vk::Format::eB8G8R8A8Srgb;
      picked_format.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    }
  } else {
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
  }

  swapchain_data_.format = picked_format.format;
  swapchain_data_.color_space = picked_format.colorSpace;

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

  vk::SwapchainCreateInfoKHR swapchain_create_info{
      {},
      *surface_,
      swapchain_data_.count,
      swapchain_data_.format,
      swapchain_data_.color_space,
      swapchain_data_.extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {},
      surface_capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      picked_mode,
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

  swapchain_data_.swapchain =
      vk::raii::SwapchainKHR{device_, swapchain_create_info};

  swapchain_data_.images = swapchain_data_.swapchain.getImages();

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
}

void Gpu::MakeCommandObjects() {
  command_pool_ =
      vk::raii::CommandPool{device_,
                            {vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                             queue_family_.graphics_index.value()}};

  vk::CommandBufferAllocateInfo command_buffer_allocate_info{
      *command_pool_, vk::CommandBufferLevel::ePrimary, frames_in_flight_};
  command_buffers_ = std::move(
      vk::raii::CommandBuffers{device_, command_buffer_allocate_info});
}

void Gpu::MakeSyncObjects() {
  fences_.reserve(frames_in_flight_);
  image_available_semaphores_.reserve(frames_in_flight_);
  render_finished_semaphores_.reserve(frames_in_flight_);

  vk::FenceCreateInfo fence_create_info{vk::FenceCreateFlagBits::eSignaled};
  vk::SemaphoreCreateInfo semaphore_create_info{};

  for (uint32_t i{0}; i < frames_in_flight_; ++i) {
    fences_.emplace_back(device_, fence_create_info);
    image_available_semaphores_.emplace_back(device_, semaphore_create_info);
    render_finished_semaphores_.emplace_back(device_, semaphore_create_info);
  }
}

void Gpu::MakeDepthImage() {
  depth_image_data_.format = vk::Format::eD32Sfloat;

  vk::ImageCreateInfo image_create_info{
      {},
      vk::ImageType::e2D,
      depth_image_data_.format,
      vk::Extent3D{swapchain_data_.extent, 1},
      1,
      1,
      sample_count_,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eDepthStencilAttachment,
      vk::SharingMode::eExclusive,
      {},
      vk::ImageLayout::eUndefined};
  depth_image_data_.image = vk::raii::Image{device_, image_create_info};

  depth_image_data_.device_memory =
      AllocateDeviceMemory(physical_device_, device_,
                           depth_image_data_.image.getMemoryRequirements(),
                           vk::MemoryPropertyFlagBits::eDeviceLocal);

  depth_image_data_.image.bindMemory(*(depth_image_data_.device_memory), 0);
  depth_image_data_.image_view = vk::raii::ImageView{
      device_,
      vk::ImageViewCreateInfo{{},
                              *(depth_image_data_.image),
                              vk::ImageViewType::e2D,
                              depth_image_data_.format,
                              {},
                              {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}}};
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

vk::raii::DeviceMemory Gpu::AllocateDeviceMemory(
    const vk::raii::PhysicalDevice& physical_device,
    const vk::raii::Device& device,
    const vk::MemoryRequirements& memory_requirements,
    vk::MemoryPropertyFlags memory_properties_flags) {
  vk::PhysicalDeviceMemoryProperties memory_properties{
      physical_device.getMemoryProperties()};

  uint32_t memory_type_bits{memory_requirements.memoryTypeBits};

  uint32_t type_index{static_cast<uint32_t>(~0)};
  for (uint32_t i{0}; i < memory_properties.memoryTypeCount; i++) {
    if ((memory_type_bits & 1) &&
        ((memory_properties.memoryTypes[i].propertyFlags &
          memory_properties_flags) == memory_properties_flags)) {
      type_index = i;
      break;
    }
    memory_type_bits >>= 1;
  }
  if (type_index == static_cast<uint32_t>(~0)) {
    THROW("Fail to find required memory.");
  }

  vk::MemoryAllocateInfo memory_allocate_info{memory_requirements.size,
                                              type_index};

  return vk::raii::DeviceMemory{device, memory_allocate_info};
}

}  // namespace luka
