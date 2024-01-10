// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-swapchain_info.format off
#include "platform/pch.h"
// clang-swapchain_info.format on

#include "context.h"
#include "function/rendering/rendering.h"

namespace luka {

Rendering::Rendering() : gpu_{gContext.gpu} {
  CreateContext();
  CreatePipeline();
}

Rendering::~Rendering() {}

void Rendering::Tick() {}

void Rendering::CreateContext() {
  rd::SwapchainInfo swapchain_info;

  // Image count.
  const vk::SurfaceCapabilitiesKHR& surface_capabilities{
      gpu_->GetSurfaceCapabilities()};
  swapchain_info.image_count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      swapchain_info.image_count > surface_capabilities.maxImageCount) {
    swapchain_info.image_count = surface_capabilities.maxImageCount;
  }

  // Format and color space.
  const std::vector<vk::SurfaceFormatKHR>& surface_formats{
      gpu_->GetSurfaceFormats()};

  vk::SurfaceFormatKHR picked_format{surface_formats[0]};

  std::vector<vk::Format> requested_formats{vk::Format::eR8G8B8A8Srgb,
                                            vk::Format::eB8G8R8A8Srgb};
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

  swapchain_info.format = picked_format.format;
  swapchain_info.color_space = picked_format.colorSpace;

  // Extent.
  if (surface_capabilities.currentExtent.width ==
      std::numeric_limits<u32>::max()) {
    i32 width{0};
    i32 height{0};
    gContext.window->GetFramebufferSize(&width, &height);

    swapchain_info.extent.width = std::clamp(
        static_cast<u32>(width), surface_capabilities.minImageExtent.width,
        surface_capabilities.maxImageExtent.width);
    swapchain_info.extent.height = std::clamp(
        static_cast<u32>(height), surface_capabilities.minImageExtent.height,
        surface_capabilities.maxImageExtent.height);
  } else {
    swapchain_info.extent = surface_capabilities.currentExtent;
  }

  // Present mode.
  std::vector<vk::PresentModeKHR> present_modes{gpu_->GetSurfacePresentModes()};

  std::vector<vk::PresentModeKHR> requested_present_modes{
      vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo,
      vk::PresentModeKHR::eImmediate};

  vk::PresentModeKHR picked_mode{vk::PresentModeKHR::eImmediate};
  for (const auto& requested_present_mode : requested_present_modes) {
    auto it{std::find_if(present_modes.begin(), present_modes.end(),
                         [requested_present_mode](const vk::PresentModeKHR& p) {
                           return p == requested_present_mode;
                         })};
    if (it != present_modes.end()) {
      picked_mode = *it;
      break;
    }
  }
  swapchain_info.present_mode = picked_mode;

  // Create swapchain
  vk::raii::SwapchainKHR swapchain{nullptr};

  vk::SwapchainCreateInfoKHR swapchain_ci{
      {},
      {},
      swapchain_info.image_count,
      swapchain_info.format,
      swapchain_info.color_space,
      swapchain_info.extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {},
      surface_capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      swapchain_info.present_mode,
      VK_TRUE,
      {}};

  const std::optional<u32>& graphics_queue_index{gpu_->GetGraphicsQueueIndex()};
  const std::optional<u32>& present_queue_index{gpu_->GetPresentQueueIndex()};

  if (graphics_queue_index.value() != present_queue_index.value()) {
    u32 queue_family_indices[2]{graphics_queue_index.value(),
                                present_queue_index.value()};
    swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queue_family_indices;
  }

  swapchain = gpu_->CreateSwapchain(swapchain_ci);

  // Create rendering frame.
  std::vector<std::unique_ptr<rd::Frame>> frames;

    std::vector<vk::Image> swapchain_images;
  swapchain_images = swapchain.getImages();

  vk::ImageViewCreateInfo swapchain_image_view_ci{
      {},
      {},
      vk::ImageViewType::e2D,
      swapchain_info.format,
      {},
      {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

  vk::ImageCreateInfo depth_image_ci{
      {},
      vk::ImageType::e2D,
      vk::Format::eD32Sfloat,
      {swapchain_info.extent.width, swapchain_info.extent.height, 1},
      1,
      1,
      vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eDepthStencilAttachment |
          vk::ImageUsageFlagBits::eTransientAttachment,
      vk::SharingMode::eExclusive,
      {},
      vk::ImageLayout::eUndefined};

  vk::ImageViewCreateInfo depth_image_view_ci{
      {},
      {},
      vk::ImageViewType::e2D,
      vk::Format::eD32Sfloat,
      {},
      {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}};

  for (auto swapchain_image : swapchain_images) {
    swapchain_image_view_ci.image = swapchain_image;
    vk::raii::ImageView swapchain_image_view{
        gpu_->CreateImageView(swapchain_image_view_ci)};

    gpu::Image depth_image{gpu_->CreateImage(depth_image_ci)};

    depth_image_view_ci.image = *depth_image;
    vk::raii::ImageView depth_image_view{
        gpu_->CreateImageView(depth_image_view_ci)};

    auto target{std::make_unique<rd::Target>(
        swapchain_image, std::move(swapchain_image_view),
        std::move(depth_image), std::move(depth_image_view))};

    auto frame{std::make_unique<rd::Frame>(std::move(target))};

    frames.push_back(std::move(frame));
  }

  // Create rendering context.
  context_ = std::make_unique<rd::Context>(
      gpu_, swapchain_info, std::move(swapchain), std::move(frames));
}

void Rendering::CreatePipeline() {}

}  // namespace luka
