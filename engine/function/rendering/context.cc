// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/context.h"

#include "function/gpu/gpu.h"

namespace luka {

namespace rd {

Context::Context(std::shared_ptr<Gpu> gpu, SwapchainInfo swapchain_info_,
                 vk::raii::SwapchainKHR&& swapchain,
                 std::vector<std::unique_ptr<Frame>>&& frames)
    : gpu_{gpu},
      swapchain_info_{swapchain_info_},
      swapchain_{std::move(swapchain)},
      frames_{std::move(frames)} {}

Context::Context(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu)
    : gpu_{gpu} {
  // Image count.
  const vk::SurfaceCapabilitiesKHR& surface_capabilities{
      gpu_->GetSurfaceCapabilities()};
  swapchain_info_.image_count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      swapchain_info_.image_count > surface_capabilities.maxImageCount) {
    swapchain_info_.image_count = surface_capabilities.maxImageCount;
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

  swapchain_info_.format = picked_format.format;
  swapchain_info_.color_space = picked_format.colorSpace;

  // Extent.
  if (surface_capabilities.currentExtent.width ==
      std::numeric_limits<u32>::max()) {
    i32 width{0};
    i32 height{0};
    window->GetFramebufferSize(&width, &height);

    swapchain_info_.extent.width = std::clamp(
        static_cast<u32>(width), surface_capabilities.minImageExtent.width,
        surface_capabilities.maxImageExtent.width);
    swapchain_info_.extent.height = std::clamp(
        static_cast<u32>(height), surface_capabilities.minImageExtent.height,
        surface_capabilities.maxImageExtent.height);
  } else {
    swapchain_info_.extent = surface_capabilities.currentExtent;
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
  swapchain_info_.present_mode = picked_mode;

  // Create swapchain
  vk::SwapchainCreateInfoKHR swapchain_ci{
      {},
      {},
      swapchain_info_.image_count,
      swapchain_info_.format,
      swapchain_info_.color_space,
      swapchain_info_.extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {},
      surface_capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      swapchain_info_.present_mode,
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

  swapchain_ = gpu_->CreateSwapchain(swapchain_ci);

  // Create rendering frame.
  std::vector<vk::Image> swapchain_images;
  swapchain_images = swapchain_.getImages();

  vk::ImageViewCreateInfo swapchain_image_view_ci{
      {},
      {},
      vk::ImageViewType::e2D,
      swapchain_info_.format,
      {},
      {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

  vk::ImageCreateInfo depth_image_ci{
      {},
      vk::ImageType::e2D,
      vk::Format::eD32Sfloat,
      {swapchain_info_.extent.width, swapchain_info_.extent.height, 1},
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
        gpu_->CreateImageView(swapchain_image_view_ci, "swapchain")};

    gpu::Image depth_image{gpu_->CreateImage(depth_image_ci, "depth")};

    depth_image_view_ci.image = *depth_image;
    vk::raii::ImageView depth_image_view{
        gpu_->CreateImageView(depth_image_view_ci, "depth")};

    auto target{std::make_unique<rd::Target>(
        swapchain_image, std::move(swapchain_image_view),
        std::move(depth_image), std::move(depth_image_view))};

    auto frame{std::make_unique<rd::Frame>(std::move(target))};

    frames_.push_back(std::move(frame));
  }
}

}  // namespace rd

}  // namespace luka
