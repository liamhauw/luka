// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/context.h"
#include "function/rendering/subpass.h"
#include "function/scene_graph/scene_graph.h"
#include "resource/asset/asset.h"

namespace luka {

namespace rd {

class Pipeline {
 public:
  Pipeline(std::vector<std::unique_ptr<rd::Subpass>>&& subpasses);

  Pipeline(std::shared_ptr<Asset> asset,
           std::shared_ptr<SceneGraph> scene_graph, Context& context);

  void Draw(Context& context);

 private:
  void BeginRenderPass(const vk::raii::CommandBuffer& command_buffer);
  void NextSubpass(const vk::raii::CommandBuffer& command_buffer);
  void EndRenderPass(const vk::raii::CommandBuffer& command_buffer);

  std::vector<std::unique_ptr<rd::Subpass>> subpasses_;

  vk::raii::RenderPass render_pass_{nullptr};
  std::vector<vk::raii::Framebuffer> framebuffers_;
  vk::raii::Pipeline pipeline_{nullptr};
};

}  // namespace rd

}  // namespace luka
