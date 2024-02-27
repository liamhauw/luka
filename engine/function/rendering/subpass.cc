// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

Subpass::Subpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                 std::shared_ptr<SceneGraph> scene_graph)
    : asset_{asset}, gpu_{gpu}, scene_graph_{scene_graph} {}

const std::vector<DrawElement>& Subpass::GetDrawElements() const {
  return draw_elements_;
}

}  // namespace rd

}  // namespace luka
