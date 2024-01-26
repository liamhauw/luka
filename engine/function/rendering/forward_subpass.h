// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/context.h"
#include "function/rendering/geometry_subpass.h"
#include "function/scene_graph/scene_graph.h"
#include "resource/asset/asset.h"

namespace luka {

namespace rd {

class ForwardSubpass : public GeometrySubpass {
 public:
  ForwardSubpass(std::shared_ptr<Asset> asset,
                 std::shared_ptr<SceneGraph> scene_graph);
  ~ForwardSubpass() = default;

  void Draw(const vk::raii::CommandBuffer& command_buffer) override;
};

}  // namespace rd

}  // namespace luka
