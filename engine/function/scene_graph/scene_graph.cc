// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene_graph.h"

namespace luka {

SceneGraph::SceneGraph(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu)
    : asset_{asset}, gpu_{gpu} {
  const AssetInfo& asset_info{asset_->GetAssetInfo()};
  const ast::Model& object{asset_info.object};

  object_ = std::move(sg::Map{gpu_, object});
  object_.LoadScene();
}

void SceneGraph::Tick() {}

}  // namespace luka
