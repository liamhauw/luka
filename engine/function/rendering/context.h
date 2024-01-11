// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/frame.h"

namespace luka {

class Gpu;

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
  Context() = default;

  Context(std::shared_ptr<Gpu> gpu, SwapchainInfo swapchain_info,
          vk::raii::SwapchainKHR&& swapchain,
          std::vector<std::unique_ptr<Frame>>&& frames_);

 private:
  std::shared_ptr<Gpu> gpu_;

  SwapchainInfo swapchain_info_;
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<std::unique_ptr<Frame>> frames_;
};

}  // namespace rd

}  // namespace luka
