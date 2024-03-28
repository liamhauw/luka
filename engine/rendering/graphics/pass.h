// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"
#include "rendering/graphics/subpass.h"
#include "resource/asset/asset.h"
#include "resource/gpu/gpu.h"

namespace luka {

namespace gs {

class Pass {
 public:
  Pass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
       std::shared_ptr<Camera> camera, u32 frame_count,
       const SwapchainInfo& swapchain_info,
       const std::vector<vk::Image>& swapchain_images,
       const std::vector<ast::Pass>& ast_passes, u32 pass_index);

  void Resize(const SwapchainInfo& swapchain_info,
              const std::vector<vk::Image>& swapchain_images);

  const std::string& GetName() const;
  bool HasUi() const;

  const vk::raii::RenderPass& GetRenderPass() const;
  const vk::raii::Framebuffer& GetFramebuffer(u32 frame_index) const;
  const vk::Rect2D& GetRenderArea() const;
  const std::vector<vk::ClearValue>& GetClearValues() const;
  const std::vector<Subpass>& GetSubpasses() const;

 protected:
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateRenderArea();
  void CreateClearValues();
  void CreateSubpasses();

  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Camera> camera_;

  u32 frame_count_;
  SwapchainInfo swapchain_info_;
  std::vector<vk::Image> swapchain_images_;
  const std::vector<ast::Pass>* ast_passes_;
  u32 pass_index_;

  const ast::Pass* ast_pass_;
  std::string name_;
  bool has_ui_;

  std::vector<u32> color_attachment_counts_;
  vk::raii::RenderPass render_pass_{nullptr};

  // For every frame.
  std::vector<std::vector<gpu::Image>> images_;
  std::vector<std::vector<vk::raii::ImageView>> image_views_;
  std::vector<vk::raii::Framebuffer> framebuffers_;

  vk::Rect2D render_area_;

  std::vector<vk::ClearValue> clear_values_;

  std::vector<Subpass> subpasses_;
};

}  // namespace gs

}  // namespace luka
