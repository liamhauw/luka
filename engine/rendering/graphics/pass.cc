// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/graphics/pass.h"

namespace luka {

namespace gs {

Pass::Pass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
           std::shared_ptr<Camera> camera, const ast::Pass& ast_pass,
           const SwapchainInfo& swapchain_info,
           const std::vector<vk::Image>& swapchain_images)
    : gpu_{gpu},
      asset_{asset},
      camera_{camera},
      ast_pass_{&ast_pass},
      swapchain_info_{swapchain_info},
      swapchain_images_{swapchain_images} {
  ParseAttachmentInfos();
  CreateRenderPass();
  CreateFramebuffers();
  CreateRenderArea();
  CreateClearValues();
  CreateSubpasses();
}

void Pass::Resize(const SwapchainInfo& swapchain_info,
                  const std::vector<vk::Image>& swapchain_images) {
  swapchain_info_ = swapchain_info;
  swapchain_images_ = swapchain_images;
  CreateFramebuffers();
  CreateRenderArea();
}

// const vk::raii::RenderPass& Pass::GetRenderPass() const { return render_pass_; }

const vk::raii::Framebuffer& Pass::GetFramebuffer(u32 frame_index) const {
  return framebuffers_[frame_index];
}

const vk::Rect2D& Pass::GetRenderArea() const { return render_area_; }

const std::vector<vk::ClearValue>& Pass::GetClearValues() const {
  return clear_values_;
}

const std::vector<Subpass>& Pass::GetSubpasses() const { return subpasses_; }

void Pass::ParseAttachmentInfos() {
  const auto& ast_attachment{ast_pass_->attachments};

  auto ci{ast_attachment.find(ast::AttachmentType::kColor)};
  if (ci != ast_attachment.end()) {
    color_attachment_infos_ = &(ci->second);
    color_attachment_count_ = color_attachment_infos_->size();
  }

  auto ri{ast_attachment.find(ast::AttachmentType::kResolve)};
  if (ri != ast_attachment.end()) {
    resolve_attachment_info_ = &((ri->second)[0]);
    resolve_attachment_count_ = 1;
  }

  auto di{ast_attachment.find(ast::AttachmentType::kDepthStencil)};
  if (di != ast_attachment.end()) {
    depth_stencil_attachment_info_ = &((di->second)[0]);
    depth_stencil_attachment_count_ = 1;
  }

  attachment_count_ = color_attachment_count_ + resolve_attachment_count_ +
                      depth_stencil_attachment_count_;
}

void Pass::CreateRenderPass() {
  // std::vector<vk::AttachmentDescription> attachment_descriptions;

  // std::vector<vk::AttachmentReference> color_attachment_refs;
  // vk::AttachmentReference resolve_attachment_ref;
  // vk::AttachmentReference depth_stencil_attachment_ref;

  // vk::SubpassDescription subpass_description{{},
  //                                            vk::PipelineBindPoint::eGraphics};

  // for (u32 i{0}; i < color_attachment_count_; ++i) {
  //   const ast::AttachmentInfo& info{(*color_attachment_infos_)[i]};
  //   vk::ImageLayout final_image_layout{
  //       info.name == "swapchain_image"
  //           ? vk::ImageLayout::ePresentSrcKHR
  //           : vk::ImageLayout::eShaderReadOnlyOptimal};

  //   attachment_descriptions.emplace_back(
  //       vk::AttachmentDescriptionFlags{}, swapchain_info_.color_format,
  //       vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
  //       vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear,
  //       vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined,
  //       final_image_layout);

  //   color_attachment_refs.emplace_back(
  //       i, vk::ImageLayout::eColorAttachmentOptimal);

  //   if (i == color_attachment_count_ - 1) {
  //     subpass_description.setColorAttachments(color_attachment_refs);
  //   }
  // }

  // if (depth_stencil_attachment_count_ == 1) {
  //   attachment_descriptions.emplace_back(
  //       vk::AttachmentDescriptionFlags{}, swapchain_info_.depth_stencil_format_,
  //       vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
  //       vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear,
  //       vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined,
  //       vk::ImageLayout::eDepthStencilAttachmentOptimal);

  //   depth_stencil_attachment_ref = {
  //       color_attachment_count_ + resolve_attachment_count_,
  //       vk::ImageLayout::eDepthStencilAttachmentOptimal};

  //   subpass_description.setPDepthStencilAttachment(
  //       &depth_stencil_attachment_ref);
  // }

  // vk::SubpassDependency subpass_dependency{
  //     VK_SUBPASS_EXTERNAL,
  //     0,
  //     vk::PipelineStageFlagBits::eColorAttachmentOutput,
  //     vk::PipelineStageFlagBits::eColorAttachmentOutput,
  //     vk::AccessFlagBits::eNone,
  //     vk::AccessFlagBits::eColorAttachmentRead |
  //         vk::AccessFlagBits::eColorAttachmentWrite,
  //     vk::DependencyFlagBits::eByRegion};

  // vk::RenderPassCreateInfo render_pass_ci{
  //     {}, attachment_descriptions, subpass_description, subpass_dependency};

  render_pass_ = *(gpu_->GetUiRenderPass());
}

void Pass::CreateFramebuffers() {
  images_.clear();
  image_views_.clear();
  framebuffers_.clear();

  u32 frame_count{static_cast<u32>(swapchain_images_.size())};
  images_.resize(frame_count);
  image_views_.resize(frame_count);

  for (u32 i{0}; i < frame_count; ++i) {
    std::vector<vk::ImageView> framebuffer_image_views;
    for (u32 j{0}; j < color_attachment_count_; ++j) {
      const ast::AttachmentInfo& info{(*color_attachment_infos_)[j]};
      bool is_swapchain{info.name == "swapchain_image" ? true : false};
      // Image.
      gpu::Image color_image;
      if (is_swapchain) {
        color_image = swapchain_images_[i];
      } else {
        vk::ImageCreateInfo color_image_ci{
            {},
            vk::ImageType::e2D,
            swapchain_info_.color_format,
            {swapchain_info_.extent.width, swapchain_info_.extent.height, 1},
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive,
            {},
            vk::ImageLayout::eUndefined};
        color_image = gpu_->CreateImage(color_image_ci);
      }

      // Image view.
      vk::ImageViewCreateInfo color_image_view_ci{
          {},
          *color_image,
          vk::ImageViewType::e2D,
          swapchain_info_.color_format,
          {},
          {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
      vk::raii::ImageView color_image_view{
          gpu_->CreateImageView(color_image_view_ci)};

      framebuffer_image_views.push_back(*color_image_view);

      images_[i].push_back(std::move(color_image));
      image_views_[i].push_back(std::move(color_image_view));
    }

    if (depth_stencil_attachment_count_ == 1) {
      vk::ImageCreateInfo depth_stencil_image_ci{
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

      gpu::Image depth_stencil_image{gpu_->CreateImage(depth_stencil_image_ci)};

      vk::ImageViewCreateInfo depth_stencil_image_view_ci{
          {},
          *depth_stencil_image,
          vk::ImageViewType::e2D,
          vk::Format::eD32Sfloat,
          {},
          {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}};
      vk::raii::ImageView depth_stencil_image_view{
          gpu_->CreateImageView(depth_stencil_image_view_ci)};

      framebuffer_image_views.emplace_back(*depth_stencil_image_view);

      images_[i].push_back(std::move(depth_stencil_image));
      image_views_[i].push_back(std::move(depth_stencil_image_view));
    }

    vk::FramebufferCreateInfo framebuffer_ci{{},
                                             render_pass_,
                                             framebuffer_image_views,
                                             swapchain_info_.extent.width,
                                             swapchain_info_.extent.height,
                                             1};

    vk::raii::Framebuffer framebuffer{gpu_->CreateFramebuffer(framebuffer_ci)};

    framebuffers_.push_back(std::move(framebuffer));
  }
}

void Pass::CreateRenderArea() {
  render_area_ = vk::Rect2D{vk::Offset2D{0, 0}, swapchain_info_.extent};
}

void Pass::CreateClearValues() {
  for (u32 i{0}; i < color_attachment_count_; ++i) {
    clear_values_.emplace_back(vk::ClearColorValue{0.0F, 0.0F, 0.0F, 1.0F});
  }

  if (depth_stencil_attachment_count_ == 1) {
    clear_values_.emplace_back(vk::ClearDepthStencilValue{1.0F, 0});
  }
}

void Pass::CreateSubpasses() {
  const std::vector<ast::Subpass>& ast_subpasses{ast_pass_->subpasses};
  for (u32 i{0}; i < ast_subpasses.size(); ++i) {
    subpasses_.emplace_back(gpu_, asset_, camera_, ast_subpasses[i],
                            render_pass_, swapchain_images_.size());
  }
}

}  // namespace gs

}  // namespace luka
