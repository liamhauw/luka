// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include "base/gpu/gpu.h"
#include "base/window/window.h"
#include "core/util.h"

namespace luka {

struct SwapchainInfo {
  u32 image_count;
  vk::Format color_format;
  vk::ColorSpaceKHR color_space;
  vk::Extent2D extent;
  vk::PresentModeKHR present_mode;
};

class FunctionUi {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(FunctionUi)

  FunctionUi(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu);

  ~FunctionUi();

  void Tick();

  static void Render(const vk::raii::CommandBuffer& command_buffer);

  const SwapchainInfo& GetSwapchainInfo() const;
  const vk::raii::SwapchainKHR& GetSwapchain() const;

  vk::raii::RenderPass GetUiRenderPass();

 private:
  void CreateSwapchain();
  void CreateImgui();

  static void DestroyImgui();

  void Resize();
  static void UpdateImgui();
  void CreateUi();

  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;

  SwapchainInfo swapchain_info_;
  vk::raii::SwapchainKHR swapchain_{nullptr};

  vk::raii::RenderPass ui_render_pass_{nullptr};
};

}  // namespace luka
