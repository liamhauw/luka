// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/frame.h"
#include "function/window/window.h"
#include "function/rendering/pass.h"

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
  Context(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu);
  ~Context();

  void Resize();

  Frame& GetActiveFrame();
  
  const std::vector<std::unique_ptr<Pass>> & GetPasses() const;

  const vk::raii::CommandBuffer& Begin();

  void End(const vk::raii::CommandBuffer& command_buffer);


 private:
  void CreateSwapchain();
  void CreateFrames();
  void CreateAcquiredSemphores();

  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;

  SwapchainInfo swapchain_info_;
  vk::raii::SwapchainKHR swapchain_{nullptr};

  std::vector<Frame> frames_;
  u32 active_frame_index_{0};

  std::vector<vk::raii::Semaphore> acquired_semaphores_;
  u32 acquired_semaphore_index{0};

  std::vector<std::unique_ptr<Pass>> passes;
};

}  // namespace rd

}  // namespace luka
