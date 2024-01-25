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

class GeometrySubpass : public Subpass {
 public:
  GeometrySubpass(std::shared_ptr<Asset> asset,
                  std::shared_ptr<SceneGraph> scene_graph,
                  Context& context);
  ~GeometrySubpass() = default;


  void Draw(const vk::raii::CommandBuffer& command_buffer) override;

 private:
  std::vector<sg::Mesh* > meshes_;
};

}  // namespace rd

}  // namespace luka
