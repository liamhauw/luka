// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/swapchain_pass.h"
#include "function/scene_graph/scene_graph.h"
#include "function/window/window.h"
#include "resource/asset/asset.h"

namespace luka {

namespace rd {

class Context {
 public:
  Context(std::shared_ptr<Asset> asset, std::shared_ptr<Window> window,
          std::shared_ptr<Gpu> gpu, std::shared_ptr<SceneGraph> scene_graph);

  ~Context();

  void Resize();

  void Render();

 private:
  void CreateSwapchain();
  void CreateSyncObjects();
  void CreateCommandObjects();
  void CreateViewportAndScissor();
  void CreatePasses();

  const vk::raii::CommandBuffer& Begin();
  void TarversePasses(const vk::raii::CommandBuffer& command_buffer);
  void End(const vk::raii::CommandBuffer& command_buffer);

  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<SceneGraph> scene_graph_;

  SwapchainInfo swapchain_info_;
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<vk::Image> swapchain_images_;
  u32 frame_count_{0};

  u32 acquired_semaphore_index_{0};
  u32 active_frame_index_{0};

  std::vector<vk::raii::Semaphore> acquired_semaphores_;
  std::vector<vk::raii::Semaphore> render_finished_semaphores_;
  std::vector<vk::raii::Fence> command_finished_fences_;

  std::vector<vk::raii::CommandPool> command_pools_;
  std::vector<vk::raii::CommandBuffers> command_buffers_;

  vk::Viewport viewport_;
  vk::Rect2D scissor_;

  std::vector<std::unique_ptr<Pass>> passes_;
};

}  // namespace rd

}  // namespace luka
