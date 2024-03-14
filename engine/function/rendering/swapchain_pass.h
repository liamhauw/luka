// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"
#include "function/rendering/pass.h"
#include "function/rendering/subpass.h"
#include "resource/asset/asset.h"
#include "resource/gpu/gpu.h"

namespace luka {

namespace rd {

class SwapchainPass : public Pass {
 public:
  SwapchainPass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
                std::shared_ptr<Camera> camera,
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

  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Camera> camera_;

  SwapchainInfo swapchain_info_;
  std::vector<vk::Image> swapchain_images_;
};

}  // namespace rd

}  // namespace luka
