// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
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
  SwapchainPass(std::shared_ptr<Gpu> gpu, const SwapchainInfo& swapchain_info);
  ~SwapchainPass() = default;

 private:
  void CreateRenderPass() override;
  void CreateRenderArea() override;
  void CreateClearValues() override;
  void CreateSubpasses() override;

  std::shared_ptr<Gpu> gpu_;
  SwapchainInfo swapchain_info_;
};

}  // namespace rd

}  // namespace luka
