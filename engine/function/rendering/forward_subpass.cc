// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/forward_subpass.h"

namespace luka {

namespace rd {

ForwardSubpass::ForwardSubpass(std::shared_ptr<Asset> asset,
                               std::shared_ptr<SceneGraph> scene_graph,
                               rd::Context& context)
    : GeometrySubpass{asset, scene_graph, context} {}

void ForwardSubpass::Draw(const vk::raii::CommandBuffer& command_buffer) {}

}  // namespace rd

}  // namespace luka
