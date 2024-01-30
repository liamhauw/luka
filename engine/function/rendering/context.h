// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/frame.h"
#include "function/rendering/swapchain_pass.h"
#include "function/window/window.h"

namespace luka {

namespace rd {

class Context {
 public:
  Context(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu);

  ~Context();

  void Resize();

  void Draw();

 private:
  void CreateSwapchain();
  void CreateFrames();
  void CreatePasses();
  void CreateAcquiredSemphores();

  const vk::raii::CommandBuffer& Begin();
  void TarversePasses(const vk::raii::CommandBuffer& command_buffer);
  void End(const vk::raii::CommandBuffer& command_buffer);

  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;

  SwapchainInfo swapchain_info_;
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<vk::Image> swapchain_images_;

  u32 frame_count_{0};
  std::vector<Frame> frames_;
  u32 active_frame_index_{0};

  std::vector<vk::raii::Semaphore> acquired_semaphores_;
  u32 acquired_semaphore_index{0};

  std::vector<std::unique_ptr<Pass>> passes_;
};

}  // namespace rd

}  // namespace luka
