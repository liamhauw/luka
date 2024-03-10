// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/gpu/gpu.h"
#include "resource/scene_graph/map.h"
#include "resource/asset/asset.h"

namespace luka {

class SceneGraph {
 public:
  SceneGraph(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu);

  void Tick();

  const sg::Map& GetObjectLuka() const;

 private:
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Gpu> gpu_;

  sg::Map model_;
};

}  // namespace luka
