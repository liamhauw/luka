// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"
#include "function/rendering/subpass.h"
#include "resource/asset/asset.h"
#include "resource/gpu/gpu.h"

namespace luka {

namespace rd {

struct SwapchainInfo {
  u32 image_count;
  vk::Format color_format;
  vk::ColorSpaceKHR color_space;
  vk::Extent2D extent;
  vk::PresentModeKHR present_mode;
  vk::Format depth_stencil_format_{vk::Format::eD32Sfloat};
};

class Pass {
 public:
  Pass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
       std::shared_ptr<Camera> camera, const ast::Pass& ast_pass,
       const SwapchainInfo& swapchain_info,
       const std::vector<vk::Image>& swapchain_images);

  void Resize(const SwapchainInfo& swapchain_info,
              const std::vector<vk::Image>& swapchain_images);

  const vk::raii::RenderPass& GetRenderPass() const;
  const vk::raii::Framebuffer& GetFramebuffer(u32 frame_index) const;
  const vk::Rect2D& GetRenderArea() const;
  const std::vector<vk::ClearValue>& GetClearValues() const;
  const std::vector<Subpass>& GetSubpasses() const;

 protected:
  void ParseAttachmentInfos();
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateRenderArea();
  void CreateClearValues();
  void CreateSubpasses();

  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Camera> camera_;

  const ast::Pass* ast_pass_;
  SwapchainInfo swapchain_info_;
  std::vector<vk::Image> swapchain_images_;

  const std::vector<ast::AttachmentInfo>* color_attachment_infos_{nullptr};
  const ast::AttachmentInfo* resolve_attachment_info_{nullptr};
  const ast::AttachmentInfo* depth_stencil_attachment_info_{nullptr};
  u32 color_attachment_count_{0};
  u32 resolve_attachment_count_{0};
  u32 depth_stencil_attachment_count_{0};
  u32 attachment_count_{0};
  vk::raii::RenderPass render_pass_{nullptr};

  std::vector<std::vector<gpu::Image>> images_;
  std::vector<std::vector<vk::raii::ImageView>> image_views_;
  std::vector<vk::raii::Framebuffer> framebuffers_;

  vk::Rect2D render_area_;

  std::vector<vk::ClearValue> clear_values_;

  std::vector<Subpass> subpasses_;
};

}  // namespace rd

}  // namespace luka
