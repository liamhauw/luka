// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/frame.h"
#include "function/window/window.h"

namespace luka {

namespace rd {

struct SwapchainInfo {
  u32 image_count;
  vk::Format format;
  vk::ColorSpaceKHR color_space;
  vk::Extent2D extent;
  vk::PresentModeKHR present_mode;
};

class Context {
 public:
  Context(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu,
          const SwapchainInfo& swapchain_info,
          vk::raii::SwapchainKHR&& swapchain, std::vector<Frame>&& frames);

  Context(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu);

  void Resize();

  const vk::raii::CommandBuffer& Begin();

  Frame& GetActiveFrame();

  void End(const vk::raii::CommandBuffer& command_buffer);

 private:
  void CreateSwapchain();
  void CreateFrames();

  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;

  SwapchainInfo swapchain_info_;
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<Frame> frames_;
};

}  // namespace rd

}  // namespace luka
