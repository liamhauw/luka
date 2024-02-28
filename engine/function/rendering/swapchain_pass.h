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

class SwapchainPass : public Pass {
 public:
  SwapchainPass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                std::shared_ptr<SceneGraph> scene_graph,
                const SwapchainInfo& swapchain_info,
                const std::vector<vk::Image>& swapchain_images);
  ~SwapchainPass() = default;

  void Resize(const SwapchainInfo& swapchain_info,
              const std::vector<vk::Image>& swapchain_images) override;

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
