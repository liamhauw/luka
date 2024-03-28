// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/graphics/pass.h"

namespace luka {

namespace gs {

Pass::Pass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
           std::shared_ptr<Camera> camera, u32 frame_count,
           const ast::Pass& ast_pass, const SwapchainInfo& swapchain_info,
           const std::vector<vk::Image>& swapchain_images)
    : gpu_{gpu},
      asset_{asset},
      camera_{camera},
      frame_count_{frame_count},
      ast_pass_{&ast_pass},
      swapchain_info_{swapchain_info},
      swapchain_images_{swapchain_images},
      name_{ast_pass_->name},
      has_ui_{!swapchain_images.empty()} {
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

const std::string& Pass::GetName() const { return name_; }

bool Pass::HasUi() const { return has_ui_; }

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
  // Ui render pass has been created, just move it from gpu to renderpass.
  if (has_ui_) {
    render_pass_ = gpu_->GetUiRenderPass();
    return;
  }

  // Create renderpass.
  std::vector<vk::AttachmentDescription> attachment_descriptions;

  const std::vector<ast::Attachment>& ast_attachments{ast_pass_->attachments};
  for (const auto& ast_attachment : ast_attachments) {
    vk::Format format{ast_attachment.format};
    vk::AttachmentStoreOp store_op{ast_attachment.output
                                       ? vk::AttachmentStoreOp::eStore
                                       : vk::AttachmentStoreOp::eDontCare};
    vk::ImageLayout final_layout{ast_attachment.output
                                     ? vk::ImageLayout::eShaderReadOnlyOptimal
                                     : vk::ImageLayout::eUndefined};

    vk::AttachmentDescription attachment_description{
        {},
        format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        store_op,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        final_layout};
    attachment_descriptions.emplace_back(
        vk::AttachmentDescriptionFlags{}, format, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, store_op, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
        final_layout);
  }

  const std::vector<ast::Subpass> ast_subpasses{ast_pass_->subpasses};
  u32 subpass_count{static_cast<u32>(ast_subpasses.size())};

  std::vector<vk::SubpassDescription> subpass_descriptions(subpass_count);
  std::vector<std::vector<vk::AttachmentReference>> input_references(
      subpass_count);
  std::vector<std::vector<vk::AttachmentReference>> color_references(
      subpass_count);
  std::vector<vk::AttachmentReference> depth_stencil_references(subpass_count);

  std::vector<vk::SubpassDependency> subpass_dependencies(subpass_count - 1);

  for (u32 i{0}; i < subpass_count; ++i) {
    vk::SubpassDescription subpass_description{
        {}, vk::PipelineBindPoint::eGraphics};

    const ast::Subpass& ast_subpass{ast_subpasses[i]};
    const auto& subpass_attachments{ast_subpass.attachments};

    // Input attachments
    auto input_it{subpass_attachments.find(ast::AttachmentType::kInput)};
    if (input_it != subpass_attachments.end()) {
      const std::vector<u32> input_attachments{input_it->second};
      input_references[i].resize(input_attachments.size());

      for (u32 j{0}; j < input_attachments.size(); ++j) {
        u32 at{input_attachments[j]};
        input_references[i][j] = {at, vk::ImageLayout::eShaderReadOnlyOptimal};
      }

      subpass_description.setInputAttachments(input_references[i]);
    }

    // Color attachments.
    auto color_it{subpass_attachments.find(ast::AttachmentType::kColor)};
    if (color_it != subpass_attachments.end()) {
      const std::vector<u32> color_attachments{color_it->second};
      color_references[i].resize(color_attachments.size());

      for (u32 j{0}; j < color_attachments.size(); ++j) {
        u32 at{color_attachments[j]};
        color_references[i][j] = {at, vk::ImageLayout::eColorAttachmentOptimal};
      }

      subpass_description.setInputAttachments(color_references[i]);
    }

    // Depth stencil attachment.
    auto depth_stencil_it{
        subpass_attachments.find(ast::AttachmentType::kDepthStencil)};
    if (depth_stencil_it != subpass_attachments.end()) {
      const std::vector<u32> depth_stencil_attachments{
          depth_stencil_it->second};

      if (depth_stencil_attachments.size() != 1) {
        LOGW("Depth stencil attachment should be 1");
      }

      u32 at{depth_stencil_attachments[0]};
      depth_stencil_references[i] = {
          at, vk::ImageLayout::eDepthStencilAttachmentOptimal};

      subpass_description.setPDepthStencilAttachment(
          &depth_stencil_references[i]);
    }

    subpass_descriptions[i] = std::move(subpass_description);

    // Subpass dependencies.
    if (i >= 1) {
      subpass_dependencies[i - 1] = {
          i - 1,
          i,
          vk::PipelineStageFlagBits::eColorAttachmentOutput,
          vk::PipelineStageFlagBits::eFragmentShader,
          vk::AccessFlagBits::eColorAttachmentWrite,
          vk::AccessFlagBits::eInputAttachmentRead,
          vk::DependencyFlagBits::eByRegion};
    }
  }

  vk::RenderPassCreateInfo render_pass_ci{
      {}, attachment_descriptions, subpass_descriptions, subpass_dependencies};
}

void Pass::CreateFramebuffers() {
  images_.clear();
  image_views_.clear();
  framebuffers_.clear();

  images_.resize(frame_count_);
  image_views_.resize(frame_count_);

  for (u32 i{0}; i < frame_count_; ++i) {
    std::vector<vk::ImageView> framebuffer_image_views;

    const std::vector<ast::Attachment>& ast_attachments{ast_pass_->attachments};
    for (const auto& ast_attachment : ast_attachments) {
      bool is_swapchain{ast_attachment.name == "swapchain" ? true : false};
      gpu::Image image;
      vk::ImageAspectFlags aspect;
      if (is_swapchain) {
        image = swapchain_images_[i];
      } else {
        vk::ImageUsageFlags usage{vk::ImageUsageFlagBits::eInputAttachment};
        if (ast_attachment.format != vk::Format::eD32Sfloat) {
          usage |= vk::ImageUsageFlagBits::eColorAttachment;
          aspect = vk::ImageAspectFlagBits::eColor;
        } else {
          usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
          aspect = vk::ImageAspectFlagBits::eDepth;
        }

        vk::ImageCreateInfo image_ci{
            {},
            vk::ImageType::e2D,
            ast_attachment.format,
            {swapchain_info_.extent.width, swapchain_info_.extent.height, 1},
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            usage,
            vk::SharingMode::eExclusive,
            {},
            vk::ImageLayout::eUndefined};
        image = gpu_->CreateImage(image_ci, ast_attachment.name);
      }

      // Image view.
      vk::ImageViewCreateInfo image_view_ci{{},
                                            *image,
                                            vk::ImageViewType::e2D,
                                            ast_attachment.format,
                                            {},
                                            {aspect, 0, 1, 0, 1}};
      vk::raii::ImageView image_view{
          gpu_->CreateImageView(image_view_ci, ast_attachment.name)};

      framebuffer_image_views.push_back(*image_view);

      images_[i].push_back(std::move(image));
      image_views_[i].push_back(std::move(image_view));
    }

    vk::FramebufferCreateInfo framebuffer_ci{{},
                                             *render_pass_,
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
  const std::vector<ast::Attachment>& ast_attachments{ast_pass_->attachments};
  for (const auto& ast_attachment : ast_attachments) {
    if (ast_attachment.format != vk::Format::eD32Sfloat) {
      clear_values_.emplace_back(vk::ClearColorValue{0.0F, 0.0F, 0.0F, 1.0F});
    } else {
      clear_values_.emplace_back(vk::ClearDepthStencilValue{1.0F, 0});
    }
  }
}

void Pass::CreateSubpasses() {
  const std::vector<ast::Subpass>& ast_subpasses{ast_pass_->subpasses};
  for (u32 i{0}; i < ast_subpasses.size(); ++i) {
    subpasses_.emplace_back(gpu_, asset_, camera_, frame_count_, *render_pass_,
                            ast_subpasses[i]);
  }
}

}  // namespace gs

}  // namespace luka
