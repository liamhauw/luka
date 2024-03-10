// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/scene_graph/scene_graph.h"

namespace luka {

SceneGraph::SceneGraph(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu)
    : asset_{asset}, gpu_{gpu} {
  const ast::Model& model{asset_->GetModel()};

  model_ = std::move(sg::Map{gpu_, model, "object"});
  model_.LoadScene();
}

void SceneGraph::Tick() {}

const sg::Map& SceneGraph::GetObjectLuka() const { return model_; }

}  // namespace luka
