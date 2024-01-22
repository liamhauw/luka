// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"

#include "core/log.h"

namespace luka {

Gpu::Gpu(std::shared_ptr<Window> window) : window_{window} {
  CreateInstance();
  CreateSurface();
  CreatePhysicalDevice();
  CreateDevice();
  CreateAllocator();
  CreateCommandObjects();
  // CreateSyncObjects();
  // CreateSwapchain();
  // CreateRenderPass();
  // CreateFramebuffers();
  // CreatePipelineCache();
  // CreateDescriptorObjects();
}

Gpu::~Gpu() {
  WaitIdle();
  vmaDestroyAllocator(allocator_);
}

void Gpu::Tick() {
  if (window_->GetIconified()) {
    return;
  }
}

gpu::Buffer Gpu::CreateBuffer(const vk::BufferCreateInfo& buffer_ci,
                              const void* data, const std::string& name) {
  gpu::Buffer buffer{allocator_, buffer_ci, true};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eBuffer,
                reinterpret_cast<uint64_t>(static_cast<VkBuffer>(*buffer)),
                name, "buffer");
#endif

  void* mapped_data{buffer.Map()};
  memcpy(mapped_data, data, buffer_ci.size);
  buffer.Unmap();

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
                name, "buffer");
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
                "image");
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
                "image");
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
                name, "image_view");
#endif

  return image_view;
}

vk::raii::Sampler Gpu::CreateSampler(const vk::SamplerCreateInfo sampler_ci,
                                     const std::string& name) {
  vk::raii::Sampler sampler{device_, sampler_ci};

#ifndef NDEBUG
  SetObjectName(vk::ObjectType::eSampler,
                reinterpret_cast<uint64_t>(static_cast<VkSampler>(*sampler)),
                name, "sampler");
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
      "swapchain");
#endif

  return swapchain;
}

const std::optional<u32>& Gpu::GetGraphicsQueueIndex() const {
  return graphics_queue_index_;
}

const std::optional<u32>& Gpu::GetPresentQueueIndex() const {
  return present_queue_index_;
}

const vk::SurfaceCapabilitiesKHR& Gpu::GetSurfaceCapabilities() const {
  return surface_capabilities_;
}

const std::vector<vk::SurfaceFormatKHR>& Gpu::GetSurfaceFormats() const {
  return surface_formats_;
}

const std::vector<vk::PresentModeKHR>& Gpu::GetSurfacePresentModes() const {
  return present_modes_;
}

vk::raii::CommandBuffer Gpu::BeginTempCommandBuffer() {
  vk::CommandBufferAllocateInfo command_buffer_allocate_info{
      *command_pools_[back_buffer_index], vk::CommandBufferLevel::ePrimary, 1};

  vk::raii::CommandBuffer command_buffer{std::move(
      vk::raii::CommandBuffers{device_, command_buffer_allocate_info}.front())};

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

// vk::raii::DescriptorSetLayout Gpu::CreateDescriptorSetLayout(
//     const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
//     const std::string& name) {
//   vk::raii::DescriptorSetLayout descriptor_set_layout{device_,
//                                                       descriptor_set_layout_ci};

// #ifndef NDEBUG
//   SetObjectName(vk::ObjectType::eDescriptorSetLayout,
//                 reinterpret_cast<uint64_t>(
//                     static_cast<VkDescriptorSetLayout>(*descriptor_set_layout)),
//                 name, "descriptor_set_layout");
// #endif

//   return descriptor_set_layout;
// }

// vk::raii::PipelineLayout Gpu::CreatePipelineLayout(
//     const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
//     const std::string& name) {
//   vk::raii::PipelineLayout pipeline_layout{device_, pipeline_layout_ci};

// #ifndef NDEBUG
//   SetObjectName(vk::ObjectType::ePipelineLayout,
//                 reinterpret_cast<uint64_t>(
//                     static_cast<VkPipelineLayout>(*pipeline_layout)),
//                 name, "pipeline_layout");
// #endif

//   return pipeline_layout;
// }

// vk::raii::Pipeline Gpu::CreatePipeline(
//     const std::vector<u8>& vertex_shader_buffer,
//     const std::vector<u8>& fragment_shader_buffer,
//     const std::vector<std::pair<u32, vk::Format>>&
//     vertex_input_stride_format, const
//     std::vector<vk::raii::DescriptorSetLayout>& descriptor_set_layout, const
//     vk::PipelineRenderingCreateInfo& pipeline_rendering_ci, const
//     std::string& name) {
//   vk::ShaderModuleCreateInfo vertex_shader_module_ci{
//       {},
//       vertex_shader_buffer.size(),
//       reinterpret_cast<const u32*>(vertex_shader_buffer.data())};
//   vk::ShaderModuleCreateInfo fragment_shader_module_ci{
//       {},
//       fragment_shader_buffer.size(),
//       reinterpret_cast<const u32*>(fragment_shader_buffer.data())};

//   vk::raii::ShaderModule vertex_shader_module{device_,
//   vertex_shader_module_ci}; vk::raii::ShaderModule
//   fragment_shader_module{device_,
//                                                 fragment_shader_module_ci};

//   std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis{
//       {{},
//        vk::ShaderStageFlagBits::eVertex,
//        *vertex_shader_module,
//        "main",
//        nullptr},
//       {{},
//        vk::ShaderStageFlagBits::eFragment,
//        *fragment_shader_module,
//        "main",
//        nullptr}};

//   vk::PipelineVertexInputStateCreateInfo vertex_input_state_ci;

//   std::vector<vk::VertexInputBindingDescription>
//       vertex_input_binding_descriptions;
//   std::vector<vk::VertexInputAttributeDescription>
//       vertex_input_attribute_descriptions;

//   for (u32 i = 0; i < vertex_input_stride_format.size(); ++i) {
//     vertex_input_binding_descriptions.emplace_back(
//         i, vertex_input_stride_format[i].first);
//     vertex_input_attribute_descriptions.emplace_back(
//         i, i, vertex_input_stride_format[i].second, 0);
//   }

//   vertex_input_state_ci.setVertexBindingDescriptions(
//       vertex_input_binding_descriptions);
//   vertex_input_state_ci.setVertexAttributeDescriptions(
//       vertex_input_attribute_descriptions);

//   vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_ci{
//       {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE};

//   vk::PipelineViewportStateCreateInfo viewport_state_ci{
//       {}, 1, nullptr, 1, nullptr};

//   vk::PipelineRasterizationStateCreateInfo rasterization_state_ci{
//       {},
//       VK_FALSE,
//       VK_FALSE,
//       vk::PolygonMode::eFill,
//       vk::CullModeFlagBits::eBack,
//       vk::FrontFace::eCounterClockwise,
//       VK_FALSE,
//       0.0f,
//       0.0f,
//       0.0f,
//       1.0f};

//   vk::PipelineMultisampleStateCreateInfo multisample_state_ci{
//       {}, vk::SampleCountFlagBits::e1};

//   vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_ci{
//       {}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE,
//   };

//   vk::ColorComponentFlags color_component_flags{
//       vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
//       vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
//   vk::PipelineColorBlendAttachmentState color_blend_attachment_state{
//       VK_FALSE,          vk::BlendFactor::eZero, vk::BlendFactor::eZero,
//       vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
//       vk::BlendOp::eAdd, color_component_flags};

//   vk::PipelineColorBlendStateCreateInfo color_blend_state_ci{
//       {},
//       VK_FALSE,
//       {},
//       color_blend_attachment_state,
//       {{0.0f, 0.0f, 0.0f, 0.0f}}};

//   std::array<vk::DynamicState, 2> dynamic_states{vk::DynamicState::eViewport,
//                                                  vk::DynamicState::eScissor};
//   vk::PipelineDynamicStateCreateInfo dynamic_state_ci{{}, dynamic_states};

//   std::vector<vk::DescriptorSetLayout> descriptor_set_layout_with_bindless;
//   for (u32 i{0}; i < descriptor_set_layout.size(); ++i) {
//     descriptor_set_layout_with_bindless.push_back(*descriptor_set_layout[i]);
//   }
//   descriptor_set_layout_with_bindless.push_back(
//       *bindless_descriptor_set_layout_);

//   vk::PipelineLayoutCreateInfo pipeline_layout_ci{
//       {}, descriptor_set_layout_with_bindless};
//   pipeline_layout_ = CreatePipelineLayout(pipeline_layout_ci);

//   vk::StructureChain<vk::GraphicsPipelineCreateInfo,
//                      vk::PipelineRenderingCreateInfo>
//       graphics_pipeline_ci{{{},
//                             shader_stage_cis,
//                             &vertex_input_state_ci,
//                             &input_assembly_state_ci,
//                             nullptr,
//                             &viewport_state_ci,
//                             &rasterization_state_ci,
//                             &multisample_state_ci,
//                             &depth_stencil_state_ci,
//                             &color_blend_state_ci,
//                             &dynamic_state_ci,
//                             *pipeline_layout_,
//                             nullptr},
//                            pipeline_rendering_ci};

//   vk::raii::Pipeline pipeline{
//       device_, nullptr,
//       graphics_pipeline_ci.get<vk::GraphicsPipelineCreateInfo>()};

// #ifndef NDEBUG
//   SetObjectName(vk::ObjectType::ePipeline,
//                 reinterpret_cast<uint64_t>(static_cast<VkPipeline>(*pipeline)),
//                 name, "pipeline");
// #endif

//   return pipeline;
// }

// vk::raii::DescriptorSet Gpu::AllocateDescriptorSet(
//     vk::DescriptorSetAllocateInfo descriptor_set_allocate_info,
//     const std::string& name) {
//   descriptor_set_allocate_info.descriptorPool = *descriptor_pool_;
//   vk::raii::DescriptorSet descriptor_set{std::move(
//       device_.allocateDescriptorSets(descriptor_set_allocate_info).front())};

// #ifndef NDEBUG
//   SetObjectName(
//       vk::ObjectType::eDescriptorSet,
//       reinterpret_cast<uint64_t>(static_cast<VkDescriptorSet>(*descriptor_set)),
//       name, "descriptor_set");
// #endif

//   return descriptor_set;
// }

// void Gpu::UpdateDescriptorSets(
//     const std::vector<vk::WriteDescriptorSet>& writes) {
//   device_.updateDescriptorSets(writes, nullptr);
// }

// const vk::raii::CommandBuffer& Gpu::BeginFrame() {
//   used_command_buffer_counts_[back_buffer_index] = 0;

//   if (device_.waitForFences(*(command_executed_fences_[back_buffer_index]),
//                             VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
//     THROW("Fail to wait for fences.");
//   }

//   // vk::Result result;
//   // std::tie(result, image_index_) = swapchain_.acquireNextImage(
//   //     UINT64_MAX, *image_available_semaphores_[back_buffer_index],
//   nullptr);
//   // if (result != vk::Result::eSuccess) {
//   //   THROW("Fail to acqurie next image.");
//   // }

//   device_.resetFences(*(command_executed_fences_[back_buffer_index]));

//   u32 index{used_command_buffer_counts_[back_buffer_index]};
//   if (index >= kMaxUsedCommandBufferCountperFrame) {
//     THROW("Fail to get command buffer.");
//   }
//   ++used_command_buffer_counts_[back_buffer_index];

//   const vk::raii::CommandBuffer& command_buffer{
//       command_buffers_[back_buffer_index][index]};
//   command_buffer.reset({});
//   command_buffer.begin({});

//   return command_buffer;
// }

// void Gpu::EndFrame(const vk::raii::CommandBuffer& command_buffer) {
// #ifndef NDEBUG
//   command_buffer.endDebugUtilsLabelEXT();
// #endif
//   command_buffer.end();
//   vk::PipelineStageFlags wait_stage{
//       vk::PipelineStageFlagBits::eColorAttachmentOutput};
//   vk::SubmitInfo
//   submit_info{*(image_available_semaphores_[back_buffer_index]),
//                              wait_stage, *command_buffer,
//                              *(render_finished_semaphores_[back_buffer_index])};
//   graphics_queue_.submit(submit_info,
//                          *(command_executed_fences_[back_buffer_index]));

//   vk::PresentInfoKHR presten_info_khr{
//       *(render_finished_semaphores_[back_buffer_index]), *swapchain_,
//       image_index_};

//   vk::Result result{present_queue_.presentKHR(presten_info_khr)};
//   if (result != vk::Result::eSuccess) {
//     THROW("Fail to present.");
//   }

//   back_buffer_index = (back_buffer_index + 1) % kBackBufferCount;
// }

// void Gpu::BeginLabel(const vk::raii::CommandBuffer& command_buffer,
//                      const std::string& label,
//                      const std::array<f32, 4>& color) {
//   vk::DebugUtilsLabelEXT debug_utils_label{label.c_str(), color};
//   command_buffer.beginDebugUtilsLabelEXT(debug_utils_label);
// }

// void Gpu::EndLabel(const vk::raii::CommandBuffer& command_buffer) {
//   command_buffer.endDebugUtilsLabelEXT();
// }

// void Gpu::BeginRenderPass(const vk::raii::CommandBuffer& command_buffer) {
//   std::array<vk::ClearValue, 1> clear_values;
//   clear_values[0].color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};

//   vk::RenderPassBeginInfo render_pass_begin_info{
//       *render_pass_, *framebuffers_[image_index_],
//       vk::Rect2D{vk::Offset2D{0, 0}, extent_}, clear_values};

//   command_buffer.beginRenderPass(render_pass_begin_info,
//                                  vk::SubpassContents::eInline);
// }

// void Gpu::EndRenderPass(const vk::raii::CommandBuffer& command_buffer) {
//   command_buffer.endRenderPass();
// }

// const u32 Gpu::GetBackBufferCount() const { return kBackBufferCount; }

// const vk::Extent2D& Gpu::GetExtent2D() const { return extent_; }

// const vk::raii::DescriptorSet& Gpu::GetBindlessDescriptorSet() const {
//   return bindless_descriptor_set;
// }

// const vk::raii::PipelineLayout& Gpu::GetPipelineLayout() const {
//   return pipeline_layout_;
// }

// std::pair<ImGui_ImplVulkan_InitInfo, VkRenderPass>
// Gpu::GetVulkanInfoForImgui()
//     const {
//   ImGui_ImplVulkan_InitInfo init_info{
//       .Instance = static_cast<VkInstance>(*instance_),
//       .PhysicalDevice = static_cast<VkPhysicalDevice>(*physical_device_),
//       .Device = static_cast<VkDevice>(*device_),
//       .QueueFamily = graphics_queue_index_.value(),
//       .Queue = static_cast<VkQueue>(*graphics_queue_),
//       .PipelineCache = static_cast<VkPipelineCache>(*pipeline_cache_),
//       .DescriptorPool = static_cast<VkDescriptorPool>(*descriptor_pool_),
//       .Subpass = 0,
//       .MinImageCount = image_count_,
//       .ImageCount = image_count_,
//       .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
//   };

//   return std::make_pair(init_info, static_cast<VkRenderPass>(*render_pass_));
// }

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

  surface_capabilities_ = physical_device_.getSurfaceCapabilitiesKHR(*surface_);
  surface_formats_ = physical_device_.getSurfaceFormatsKHR(*surface_);
  present_modes_ = physical_device_.getSurfacePresentModesKHR(*surface_);
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
  dynamic_rendering_features.pNext = &indexing_features;
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

void Gpu::SetObjectName(vk::ObjectType object_type, u64 handle,
                        const std::string& name, const std::string& suffix) {
  if (!name.empty()) {
    std::string name_suffix{name + "_" + suffix};
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

// void Gpu::CreateSyncObjects() {
//   command_executed_fences_.reserve(kBackBufferCount);
//   image_available_semaphores_.reserve(kBackBufferCount);
//   render_finished_semaphores_.reserve(kBackBufferCount);

//   vk::FenceCreateInfo fence_ci{vk::FenceCreateFlagBits::eSignaled};
//   vk::SemaphoreCreateInfo semaphore_ci{};

//   for (u32 i{0}; i < kBackBufferCount; ++i) {
//     command_executed_fences_.emplace_back(device_, fence_ci);
//     image_available_semaphores_.emplace_back(device_, semaphore_ci);
//     render_finished_semaphores_.emplace_back(device_, semaphore_ci);
//   }
// }

// void Gpu::CreateSwapchain() {
//   // Image count.
//   vk::SurfaceCapabilitiesKHR surface_capabilities{
//       physical_device_.getSurfaceCapabilitiesKHR(*surface_)};
//   image_count_ = surface_capabilities.minImageCount + 1;
//   if (surface_capabilities.maxImageCount > 0 &&
//       image_count_ > surface_capabilities.maxImageCount) {
//     image_count_ = surface_capabilities.maxImageCount;
//   }

//   // Format and color space.
//   std::vector<vk::SurfaceFormatKHR> surface_formats{
//       physical_device_.getSurfaceFormatsKHR(*(surface_))};

//   vk::SurfaceFormatKHR picked_format{surface_formats[0]};

//   std::vector<vk::Format> requested_formats{vk::Format::eR8G8B8A8Srgb,
//                                             vk::Format::eB8G8R8A8Srgb};
//   vk::ColorSpaceKHR requested_color_space{vk::ColorSpaceKHR::eSrgbNonlinear};
//   for (const auto& requested_format : requested_formats) {
//     auto it{std::find_if(surface_formats.begin(), surface_formats.end(),
//                          [requested_format, requested_color_space](
//                              const vk::SurfaceFormatKHR& f) {
//                            return (f.format == requested_format) &&
//                                   (f.colorSpace == requested_color_space);
//                          })};
//     if (it != surface_formats.end()) {
//       picked_format = *it;
//       break;
//     }
//   }

//   format_ = picked_format.format;
//   color_space_ = picked_format.colorSpace;

//   // Extent.
//   if (surface_capabilities.currentExtent.width ==
//       std::numeric_limits<u32>::max()) {
//     i32 width{0};
//     i32 height{0};
//     window_->GetFramebufferSize(&width, &height);

//     extent_.width = std::clamp(static_cast<u32>(width),
//                                surface_capabilities.minImageExtent.width,
//                                surface_capabilities.maxImageExtent.width);
//     extent_.height = std::clamp(static_cast<u32>(height),
//                                 surface_capabilities.minImageExtent.height,
//                                 surface_capabilities.maxImageExtent.height);
//   } else {
//     extent_ = surface_capabilities.currentExtent;
//   }

//   // Present mode.
//   std::vector<vk::PresentModeKHR> present_modes{
//       physical_device_.getSurfacePresentModesKHR(*surface_)};

//   std::vector<vk::PresentModeKHR> requested_present_modes{
//       vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo,
//       vk::PresentModeKHR::eImmediate};

//   vk::PresentModeKHR picked_mode{vk::PresentModeKHR::eImmediate};
//   for (const auto& requested_present_mode : requested_present_modes) {
//     auto it{std::find_if(present_modes.begin(), present_modes.end(),
//                          [requested_present_mode](const vk::PresentModeKHR&
//                          p) {
//                            return p == requested_present_mode;
//                          })};
//     if (it != present_modes.end()) {
//       picked_mode = *it;
//       break;
//     }
//   }
//   present_mode_ = picked_mode;

//   // Create swapchain
//   vk::SwapchainCreateInfoKHR swapchain_ci{
//       {},
//       *surface_,
//       image_count_,
//       format_,
//       color_space_,
//       extent_,
//       1,
//       vk::ImageUsageFlagBits::eColorAttachment,
//       vk::SharingMode::eExclusive,
//       {},
//       surface_capabilities.currentTransform,
//       vk::CompositeAlphaFlagBitsKHR::eOpaque,
//       present_mode_,
//       VK_TRUE,
//       {}};

//   if (graphics_queue_index_.value() != present_queue_index_.value()) {
//     u32 queue_family_indices[2]{graphics_queue_index_.value(),
//                                 present_queue_index_.value()};
//     swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
//     swapchain_ci.queueFamilyIndexCount = 2;
//     swapchain_ci.pQueueFamilyIndices = queue_family_indices;
//   }

//   swapchain_.clear();
//   swapchain_ = vk::raii::SwapchainKHR{device_, swapchain_ci};

//   // Get swapchain images.
//   images_ = swapchain_.getImages();

//   // Create swapchain image views
//   image_views_.clear();
//   image_views_.reserve(images_.size());
//   vk::ImageViewCreateInfo image_view_ci{
//       {},      {}, vk::ImageViewType::e2D,
//       format_, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
//   for (auto image : images_) {
//     image_view_ci.image = image;
//     image_views_.emplace_back(device_, image_view_ci);
//   }
// }

// void Gpu::CreateRenderPass() {
//   std::vector<vk::AttachmentDescription> attachment_descriptions;

//   attachment_descriptions.emplace_back(
//       vk::AttachmentDescriptionFlags{}, format_, vk::SampleCountFlagBits::e1,
//       vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
//       vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
//       vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

//   vk::AttachmentReference color_attachment_ref{
//       0, vk::ImageLayout::eColorAttachmentOptimal};

//   vk::SubpassDescription subpass_description{
//       {}, vk::PipelineBindPoint::eGraphics, {}, color_attachment_ref};

//   vk::SubpassDependency subpass_dependency{
//       VK_SUBPASS_EXTERNAL,
//       0,
//       vk::PipelineStageFlagBits::eColorAttachmentOutput,
//       vk::PipelineStageFlagBits::eColorAttachmentOutput,
//       {},
//       vk::AccessFlagBits::eColorAttachmentRead |
//           vk::AccessFlagBits::eColorAttachmentWrite};

//   vk::RenderPassCreateInfo render_pass_ci{
//       {}, attachment_descriptions, subpass_description, subpass_dependency};

//   render_pass_ = vk::raii::RenderPass{device_, render_pass_ci};
// }

// void Gpu::CreateFramebuffers() {
//   std::vector<vk::ImageView> attachements(1);
//   vk::FramebufferCreateInfo framebuffer_ci{
//       {}, *render_pass_, attachements, extent_.width, extent_.height, 1};

//   framebuffers_.clear();
//   framebuffers_.reserve(image_views_.size());
//   for (const auto& image_view : image_views_) {
//     attachements[0] = *image_view;
//     framebuffers_.emplace_back(device_, framebuffer_ci);
//   }
// }

// void Gpu::CreatePipelineCache() {
//   vk::PipelineCacheCreateInfo pipeline_cache_ci;
//   pipeline_cache_ = vk::raii::PipelineCache{device_, pipeline_cache_ci};
// }

// void Gpu::CreateDescriptorObjects() {
//   // Global descriptor pool.
//   {
//     std::vector<vk::DescriptorPoolSize> pool_sizes{
//         {vk::DescriptorType::eSampler, 1000},
//         {vk::DescriptorType::eCombinedImageSampler, 1000},
//         {vk::DescriptorType::eSampledImage, 1000},
//         {vk::DescriptorType::eStorageImage, 1000},
//         {vk::DescriptorType::eUniformTexelBuffer, 1000},
//         {vk::DescriptorType::eStorageTexelBuffer, 1000},
//         {vk::DescriptorType::eUniformBuffer, 1000},
//         {vk::DescriptorType::eStorageBuffer, 1000},
//         {vk::DescriptorType::eUniformBufferDynamic, 1000},
//         {vk::DescriptorType::eStorageBufferDynamic, 1000},
//         {vk::DescriptorType::eInputAttachment, 1000}};

//     u32 max_sets{std::accumulate(
//         pool_sizes.begin(), pool_sizes.end(), static_cast<u32>(0),
//         [](u32 sum, const vk::DescriptorPoolSize& dps) {
//           return sum + dps.descriptorCount;
//         })};

//     vk::DescriptorPoolCreateInfo descriptor_pool_ci{
//         vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, max_sets,
//         pool_sizes};
//     descriptor_pool_ = vk::raii::DescriptorPool{device_, descriptor_pool_ci};
//   }

//   // Bindless descriptor objects.
//   {
//     std::vector<vk::DescriptorPoolSize> bindlessl_pool_sizes{
//         {vk::DescriptorType::eCombinedImageSampler,
//         kBindlessDescriptorCount}, {vk::DescriptorType::eStorageImage,
//         kBindlessDescriptorCount}};

//     u32 bindless_max_sets{std::accumulate(
//         bindlessl_pool_sizes.begin(), bindlessl_pool_sizes.end(),
//         static_cast<u32>(0), [](u32 sum, const vk::DescriptorPoolSize& dps) {
//           return sum + dps.descriptorCount;
//         })};

//     vk::DescriptorPoolCreateInfo bindless_descriptor_pool_ci{
//         vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind |
//             vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
//         bindless_max_sets, bindlessl_pool_sizes};
//     bindless_descriptor_pool_ =
//         vk::raii::DescriptorPool{device_, bindless_descriptor_pool_ci};
//     std::vector<vk::DescriptorSetLayoutBinding>
//     descriptor_set_layout_bindings{
//         {kBindlessBinding, vk::DescriptorType::eCombinedImageSampler,
//          kBindlessDescriptorCount, vk::ShaderStageFlagBits::eAll},
//         {kBindlessBinding + 1, vk::DescriptorType::eCombinedImageSampler,
//          kBindlessDescriptorCount, vk::ShaderStageFlagBits::eAll},
//     };

//     std::vector<vk::DescriptorBindingFlags> descriptor_binding_flags(
//         2, vk::DescriptorBindingFlagBits::ePartiallyBound);

//     vk::DescriptorSetLayoutBindingFlagsCreateInfo
//         descriptor_set_layout_binding_flags_ci{descriptor_binding_flags};

//     vk::DescriptorSetLayoutCreateInfo bindless_descriptor_set_layout_ci{
//         vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
//         descriptor_set_layout_bindings,
//         &descriptor_set_layout_binding_flags_ci};

//     bindless_descriptor_set_layout_ = vk::raii::DescriptorSetLayout{
//         device_, bindless_descriptor_set_layout_ci};

//     vk::DescriptorSetAllocateInfo bindless_descriptor_set_allocate_info{
//         *bindless_descriptor_pool_, *bindless_descriptor_set_layout_};

//     bindless_descriptor_set = std::move(
//         device_.allocateDescriptorSets(bindless_descriptor_set_allocate_info)
//             .front());
//   }
// }

// void Gpu::Resize() {
//   CreateSwapchain();
//   CreateFramebuffers();
// }

// const vk::raii::CommandBuffer& Gpu::GetCommandBuffer() {
//   u32 index{used_command_buffer_counts_[back_buffer_index]};
//   if (index >= kMaxUsedCommandBufferCountperFrame) {
//     THROW("Fail to get command buffer.");
//   }
//   ++used_command_buffer_counts_[back_buffer_index];

//   return command_buffers_[back_buffer_index][index];
// }

}  // namespace luka
