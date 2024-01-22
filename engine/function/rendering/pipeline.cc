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
      std::make_unique<rd::ForwardSubpass>(asset, scene_graph, context)};
  subpasses_.push_back(std::move(forward_subpass));
}

}  // namespace rd

}  // namespace luka
