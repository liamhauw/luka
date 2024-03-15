// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/pass.h"

namespace luka {

namespace rd {

Pass::Pass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
           std::shared_ptr<Camera> camera, const ast::Pass& ast_pass,
           const SwapchainInfo& swapchain_info,
           const std::vector<vk::Image>& swapchain_images)
    : gpu_{gpu},
      asset_{asset},
      camera_{camera},
      ast_pass_{&ast_pass},
      swapchain_info_{swapchain_info},
      swapchain_images_{swapchain_images},
      attachment_count_{2},
      color_attachment_indices_{0},
      resolve_attachment_indices_{},
      depth_stencil_attachment_index_{1} {
  CreateRenderPass();
  CreateFramebuffers();
  CreateRenderArea();
  CreateClearValues();
  CreateSubpasses();
}

const vk::raii::RenderPass& Pass::GetRenderPass() const { return render_pass_; }

const vk::raii::Framebuffer& Pass::GetFramebuffer(u32 frame_index) const {
  return framebuffers_[frame_index];
}

const vk::Rect2D& Pass::GetRenderArea() const { return render_area_; }

const std::vector<vk::ClearValue>& Pass::GetClearValues() const {
  return clear_values_;
}

const std::vector<Subpass>& Pass::GetSubpasses() const { return subpasses_; }

void Pass::CreateRenderPass() {
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

void Pass::CreateFramebuffers() {
  images_.clear();
  image_views_.clear();
  framebuffers_.clear();

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

  u32 frame_count{static_cast<u32>(swapchain_images_.size())};
  images_.resize(frame_count);
  image_views_.resize(frame_count);

  for (u32 i{0}; i < frame_count; ++i) {
    gpu::Image swapchain_image_gpu{swapchain_images_[i]};

    swapchain_image_view_ci.image = *swapchain_image_gpu;
    vk::raii::ImageView swapchain_image_view{
        gpu_->CreateImageView(swapchain_image_view_ci, "swapchain")};

    gpu::Image depth_image{gpu_->CreateImage(depth_image_ci, "depth")};

    depth_image_view_ci.image = *depth_image;
    vk::raii::ImageView depth_image_view{
        gpu_->CreateImageView(depth_image_view_ci, "depth")};

    std::vector<vk::ImageView> framebuffer_image_views{*swapchain_image_view,
                                                       *depth_image_view};
    vk::FramebufferCreateInfo framebuffer_ci{{},
                                             *render_pass_,
                                             framebuffer_image_views,
                                             swapchain_info_.extent.width,
                                             swapchain_info_.extent.height,
                                             1};

    vk::raii::Framebuffer framebuffer{gpu_->CreateFramebuffer(framebuffer_ci)};

    images_[i].push_back(std::move(swapchain_image_gpu));
    images_[i].push_back(std::move(depth_image));

    image_views_[i].push_back(std::move(swapchain_image_view));
    image_views_[i].push_back(std::move(depth_image_view));

    framebuffers_.push_back(std::move(framebuffer));
  }
}

void Pass::CreateRenderArea() {
  render_area_ = vk::Rect2D{vk::Offset2D{0, 0}, swapchain_info_.extent};
}

void Pass::CreateClearValues() {
  clear_values_.resize(attachment_count_);
  clear_values_[color_attachment_indices_[0]].color =
      vk::ClearColorValue{0.0F, 0.0F, 0.0F, 1.0F};
  clear_values_[depth_stencil_attachment_index_].depthStencil =
      vk::ClearDepthStencilValue{1.0F, 0};
}

void Pass::CreateSubpasses() {
  const std::vector<ast::Subpass>& ast_subpasses{ast_pass_->subpasses};
  for (u32 i{0}; i < ast_subpasses.size(); ++i) {
    subpasses_.emplace_back(gpu_, asset_, camera_, ast_subpasses[i],
                            render_pass_, swapchain_images_.size());
  }
}

}  // namespace rd

}  // namespace luka
