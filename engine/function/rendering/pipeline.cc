// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/pipeline.h"

#include "function/rendering/forward_subpass.h"

namespace luka {

namespace rd {

Pipeline::Pipeline(std::vector<std::unique_ptr<rd::Subpass>>&& subpasses)
    : subpasses_{std::move(subpasses)} {}

Pipeline::Pipeline(std::shared_ptr<Asset> asset,
                   std::shared_ptr<SceneGraph> scene_graph,
                   rd::Context& context) {
  auto forward_subpass{
      std::make_unique<rd::ForwardSubpass>(asset, scene_graph)};
  subpasses_.push_back(std::move(forward_subpass));
}

void Pipeline::Draw(Context& context) {
  const vk::raii::CommandBuffer& command_buffer{context.Begin()};
  const rd::Target& target = context.GetActiveFrame().GetTarget();

  for (u32 i{0}; i < subpasses_.size(); ++i) {
    if (i == 0) {
      BeginRenderPass(command_buffer);
    } else {
      NextSubpass(command_buffer);
    }
    subpasses_[i]->Draw(command_buffer);
  }
  EndRenderPass(command_buffer);

  context.End(command_buffer);
}

void Pipeline::BeginRenderPass(const vk::raii::CommandBuffer& command_buffer) {
  vk::RenderPassBeginInfo render_pass_bi{
      *render_pass_,
  };
}

void Pipeline::NextSubpass(const vk::raii::CommandBuffer& command_buffer) {}

void Pipeline::EndRenderPass(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.endRenderPass();
}

}  // namespace rd

}  // namespace luka
