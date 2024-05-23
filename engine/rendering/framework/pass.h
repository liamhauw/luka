// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/gpu/gpu.h"
#include "function/camera/camera.h"
#include "function/function_ui/function_ui.h"
#include "rendering/framework/subpass.h"
#include "resource/asset/asset.h"

namespace luka::fw {

class Pass {
 public:
  Pass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
       std::shared_ptr<Camera> camera, std::shared_ptr<FunctionUi> function_ui,
       u32 frame_count, const SwapchainInfo& swapchain_info,
       const std::vector<vk::Image>& swapchain_images,
       const std::vector<ast::Pass>& ast_passes, u32 pass_index,
       std::vector<std::unordered_map<std::string, vk::ImageView>>&
           shared_image_views,
       const std::vector<ScenePrimitive>& scene_primitives);

  void Resize(const SwapchainInfo& swapchain_info,
              const std::vector<vk::Image>& swapchain_images);

  std::vector<Subpass>& GetSubpasses();
  const std::string& GetName() const;
  vk::RenderPassBeginInfo GetRenderPassBeginInfo(u32 frame_index) const;
  const std::vector<Subpass>& GetSubpasses() const;
  bool HasUi() const;

 protected:
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateRenderArea();
  void CreateClearValues();
  void CreateSubpasses();

  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<FunctionUi> function_ui_;

  u32 frame_count_{};
  SwapchainInfo swapchain_info_{};
  std::vector<vk::Image> swapchain_images_;
  const std::vector<ast::Pass>* ast_passes_{};
  u32 pass_index_{};
  std::vector<std::unordered_map<std::string, vk::ImageView>>*
      shared_image_views_;
  const std::vector<ScenePrimitive>* scene_primitives_;

  const ast::Pass* ast_pass_{};
  std::string name_;
  bool has_ui_{};

  std::vector<u32> color_attachment_counts_;
  vk::raii::RenderPass render_pass_{nullptr};

  std::vector<std::vector<gpu::Image>> images_;
  std::vector<std::vector<vk::raii::ImageView>> image_views_;
  std::vector<vk::raii::Framebuffer> framebuffers_;

  vk::Rect2D render_area_;

  std::vector<vk::ClearValue> clear_values_;

  std::vector<Subpass> subpasses_;
};

}  // namespace luka::fw
