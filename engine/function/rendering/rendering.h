// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"
#include "resource/gpu/gpu.h"
#include "function/rendering/context.h"
#include "resource/asset/scene_component/scene_graph.h"
#include "resource/window/window.h"
#include "resource/asset/asset.h"

namespace luka {

class Rendering {
 public:
  Rendering(std::shared_ptr<Asset> asset, std::shared_ptr<Window> window,
            std::shared_ptr<Camera> camera, std::shared_ptr<Gpu> gpu,
            std::shared_ptr<SceneGraph> scene_graph);

  ~Rendering();

  void Tick();

 private:
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Window> window_;
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<SceneGraph> scene_graph_;

  rd::Context context_;
};

}  // namespace luka
