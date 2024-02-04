// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/swapchain_pass.h"

namespace luka {

namespace rd {
SwapchainPass::SwapchainPass(std::shared_ptr<Gpu> gpu,
                             std::vector<Frame>& frames,
                             const SwapchainInfo& swapchain_info,
                             const std::vector<vk::Image>& swapchain_images)
    : Pass{gpu, frames, 2, {0}, {}, 1},
      swapchain_info_{swapchain_info},
      swapchain_images_{swapchain_images} {
  CreateRenderPass();
  CreateFramebuffers();
  CreateRenderArea();
  CreateClearValues();
  CreateSubpasses();
}

void SwapchainPass::CreateRenderPass() {
  std::vector<vk::AttachmentDescription> attachment_descriptions(
      attachment_count_);

  attachment_descriptions[color_attachment_indices_[0]] =
      vk::AttachmentDescription{
          vk::AttachmentDescriptionFlags{}, swapchain_info_.color_format,
          vk::SampleCountFlagBits::e1,      vk::AttachmentLoadOp::eClear,
          vk::AttachmentStoreOp::eStore,    vk::AttachmentLoadOp::eClear,
          vk::AttachmentStoreOp::eStore,    vk::ImageLayout::eUndefined,
          vk::ImageLayout::ePresentSrcKHR};

  attachment_descriptions[depth_stencil_attachment_index_] =
      vk::AttachmentDescription{
          vk::AttachmentDescriptionFlags{},
          swapchain_info_.depth_stencil_format_,
          vk::SampleCountFlagBits::e1,
          vk::AttachmentLoadOp::eClear,
          vk::AttachmentStoreOp::eStore,
          vk::AttachmentLoadOp::eClear,
          vk::AttachmentStoreOp::eStore,
          vk::ImageLayout::eUndefined,
          vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::AttachmentReference color_attachment_ref{
      color_attachment_indices_[0], vk::ImageLayout::eColorAttachmentOptimal};
  vk::AttachmentReference depth_stencil_attachment_ref{
      depth_stencil_attachment_index_,
      vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::SubpassDescription subpass_description{
      {}, vk::PipelineBindPoint::eGraphics, {}, color_attachment_ref,
      {}, &depth_stencil_attachment_ref};

  vk::RenderPassCreateInfo render_pass_ci{
      {}, attachment_descriptions, subpass_description};

  render_pass_ = gpu_->CreateRenderPass(render_pass_ci);
}

void SwapchainPass::CreateFramebuffers() {
  vk::ImageViewCreateInfo swapchain_image_view_ci{
      {},
      {},
      vk::ImageViewType::e2D,
      swapchain_info_.color_format,
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

  for (u32 i{0}; i < frames_->size(); ++i) {
    gpu::Image swapchain_image_gpu{swapchain_images_[i]};

    swapchain_image_view_ci.image = *swapchain_image_gpu;
    vk::raii::ImageView swapchain_image_view{
        gpu_->CreateImageView(swapchain_image_view_ci, "swapchain")};

    gpu::Image depth_image{gpu_->CreateImage(depth_image_ci, "depth")};

    depth_image_view_ci.image = *depth_image;
    vk::raii::ImageView depth_image_view{
        gpu_->CreateImageView(depth_image_view_ci, "depth")};

    std::vector<vk::ImageView> framebuffer_image_views{*swapchain_image_view, *depth_image_view};
    vk::FramebufferCreateInfo framebuffer_ci{{},
                                             *render_pass_,
                                             framebuffer_image_views,
                                             swapchain_info_.extent.width,
                                             swapchain_info_.extent.height,
                                             1};

    vk::raii::Framebuffer framebuffer{gpu_->CreateFramebuffer(framebuffer_ci)};

    // std::vector<gpu::Image> images{std::move(swapchain_image_gpu),
    //                                std::move(depth_image)};
    // std::vector<vk::raii::ImageView> image_views{
    //     std::move(swapchain_image_view), std::move(depth_image_view)};
    std::vector<gpu::Image> images;
    images.push_back(std::move(swapchain_image_gpu));
    images.push_back(std::move(depth_image));
    std::vector<vk::raii::ImageView> image_views;
    image_views.push_back(std::move(swapchain_image_view));
    image_views.push_back(std::move(depth_image_view));

    images_index_ = (*frames_)[i].AddImages(std::move(images));
    image_views_index_ = (*frames_)[i].AddImageViews(std::move(image_views));
    framebuffer_index_ = (*frames_)[i].AddFramebuffer(std::move(framebuffer));
  }
}

void SwapchainPass::CreateRenderArea() {
  render_area_ = vk::Rect2D{vk::Offset2D{0, 0}, swapchain_info_.extent};
}

void SwapchainPass::CreateClearValues() {
  clear_values_.resize(attachment_count_);
  clear_values_[color_attachment_indices_[0]].color =
      vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
  clear_values_[depth_stencil_attachment_index_].depthStencil =
      vk::ClearDepthStencilValue{1.0f, 0};
}

void SwapchainPass::CreateSubpasses() {}

}  // namespace rd

}  // namespace luka
