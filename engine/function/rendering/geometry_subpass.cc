// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/geometry_subpass.h"

namespace luka {

namespace rd {

GeometrySubpass::GeometrySubpass(std::shared_ptr<Asset> asset,
                                 std::shared_ptr<SceneGraph> scene_graph,
                                 Context& context)
    : Subpass{asset, context},
      meshes_{scene_graph->GetObject().GetComponents<sg::Mesh>()} {}

}  // namespace rd

}  // namespace luka
