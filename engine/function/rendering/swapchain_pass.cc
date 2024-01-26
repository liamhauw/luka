// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/swapchain_pass.h"

namespace luka {

namespace rd {
SwapchainPass::SwapchainPass(std::shared_ptr<Gpu> gpu,
                             const SwapchainInfo& swapchain_info)
    : gpu_{gpu}, swapchain_info_{swapchain_info} {
  CreateRenderPass();
  CreateRenderArea();
  CreateClearValues();
  CreateSubpasses();
}

void SwapchainPass::CreateRenderPass() {
  std::vector<vk::AttachmentDescription> attachment_descriptions;

  attachment_descriptions.emplace_back(
      vk::AttachmentDescriptionFlags{}, swapchain_info_.color_format,
      vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined,
      vk::ImageLayout::ePresentSrcKHR);

  attachment_descriptions.emplace_back(
      vk::AttachmentDescriptionFlags{}, swapchain_info_.depth_stencil_format_,
      vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal);

  vk::AttachmentReference color_attachment_ref{
      0, vk::ImageLayout::eColorAttachmentOptimal};
  vk::AttachmentReference depth_stencil_attachment_ref{
      1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::SubpassDescription subpass_description{
      {}, vk::PipelineBindPoint::eGraphics, {}, color_attachment_ref,
      {}, &depth_stencil_attachment_ref};

  vk::RenderPassCreateInfo render_pass_ci{
      {}, attachment_descriptions, subpass_description};

  render_pass_ = gpu_->CreateRenderPass(render_pass_ci);
}

void SwapchainPass::CreateRenderArea() {
  render_area_ = vk::Rect2D{vk::Offset2D{0, 0}, swapchain_info_.extent};
}

void SwapchainPass::CreateClearValues() {
  clear_values_.resize(2);
  clear_values_[0].color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
  clear_values_[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
}

void SwapchainPass::CreateSubpasses() {}

}  // namespace rd

}  // namespace luka
