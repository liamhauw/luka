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
  MakeInstance();
  MakeSurface();
  MakePhysicalDevice();
  MakeDevice();
}

Gpu::~Gpu() { device_.waitIdle(); }

void Gpu::MakeInstance() {
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
}

void Gpu::MakeSurface() {
  VkSurfaceKHR surface;
  window_->CreateWindowSurface(instance_, &surface);
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

    // Extensions.
    bool has_extensions{true};
    std::vector<vk::ExtensionProperties> extension_properties{
        physical_device.enumerateDeviceExtensionProperties()};
    for (const char* device_extension : required_device_extensions_) {
      if (std::find_if(extension_properties.begin(), extension_properties.end(),
                       [device_extension](const vk::ExtensionProperties& ep) {
                         return (strcmp(device_extension, ep.extensionName) ==
                                 0);
                       }) == extension_properties.end()) {
        has_extensions = false;
        break;
      }
    }

    // Requirements not met.
    if (!queue_family.IsComplete() || !has_extensions) {
      continue;
    }

    // The gpu with the highest score is selected.
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
      queue_family_ = queue_family;
    }
  }

  if (!(*physical_device_)) {
    THROW("Fail to find physical device.");
  }

  // Properties.
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

  float queue_priority{0.0f};
  for (uint32_t queue_family_index : queue_family_indexes) {
    vk::DeviceQueueCreateInfo device_queue_create_info{
        {}, queue_family_index, 1, &queue_priority};
    device_queue_create_infos.push_back(device_queue_create_info);
  }

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
  fragment_shading_rate.pipelineFragmentShadingRate = VK_TRUE;
  fragment_shading_rate.primitiveFragmentShadingRate = VK_TRUE;
  fragment_shading_rate.attachmentFragmentShadingRate = VK_TRUE;

  vk::PhysicalDeviceRobustness2FeaturesEXT robustness2;
  robustness2.nullDescriptor = VK_TRUE;

  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan12Features,
                     vk::PhysicalDeviceFragmentShadingRateFeaturesKHR,
                     vk::PhysicalDeviceRobustness2FeaturesEXT>
      physical_device_features2{physical_device_features,
                                physical_device_vulkan11_features,
                                physical_device_vulkan12_features,
                                fragment_shading_rate, robustness2};

  vk::DeviceCreateInfo device_create_info{{},      device_queue_create_infos,
                                          {},      enabled_extensions,
                                          nullptr, &physical_device_features2};

  device_ = vk::raii::Device{physical_device_, device_create_info};

  graphics_queue_ =
      vk::raii::Queue{device_, queue_family_.graphics_index.value(), 0};
  compute_queue_ =
      vk::raii::Queue{device_, queue_family_.compute_index.value(), 0};
  present_queue_ =
      vk::raii::Queue{device_, queue_family_.present_index.value(), 0};
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
