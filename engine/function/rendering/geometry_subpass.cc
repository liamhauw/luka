// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/geometry_subpass.h"

namespace luka {

namespace rd {

GeometrySubpass::GeometrySubpass(std::shared_ptr<Asset> asset,
                                 std::shared_ptr<Gpu> gpu,
                                 std::shared_ptr<SceneGraph> scene_graph)
    : Subpass{asset, gpu, scene_graph} {}

void GeometrySubpass::CreatePipeline() {
  
}

void GeometrySubpass::CreateDrawElements() {}

}  // namespace rd

}  // namespace luka
