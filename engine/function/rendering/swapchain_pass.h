// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/pass.h"
#include "function/window/window.h"

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

class SwapchainPass : public Pass {
 public:
  SwapchainPass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                std::shared_ptr<SceneGraph> scene_graph,
                std::vector<Frame>& frames, const SwapchainInfo& swapchain_info,
                const std::vector<vk::Image>& swapchain_images);
  ~SwapchainPass() = default;

 private:
  void CreateRenderPass() override;
  void CreateFramebuffers() override;
  void CreateRenderArea() override;
  void CreateClearValues() override;
  void CreateSubpasses() override;

  SwapchainInfo swapchain_info_;
  std::vector<vk::Image> swapchain_images_;
};

}  // namespace rd

}  // namespace luka
