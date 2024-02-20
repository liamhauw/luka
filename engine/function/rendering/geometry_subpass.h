// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

class GeometrySubpass : public Subpass {
 public:
  GeometrySubpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                  std::shared_ptr<SceneGraph> scene_graph);
  ~GeometrySubpass() = default;

 private:
  void CreatePipeline() override;
  void CreateDrawElements() override;
};

}  // namespace rd

}  // namespace luka
