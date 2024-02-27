// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/spirv.h"
#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

class GeometrySubpass : public Subpass {
 public:
  GeometrySubpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                  std::shared_ptr<SceneGraph> scene_graph,
                  const vk::raii::RenderPass& render_pass);
  ~GeometrySubpass() = default;

 private:
  void CreateDrawElements(const vk::raii::RenderPass& render_pass) override;

  DrawElement CreateDrawElement(const sg::Primitive& primitive, const vk::raii::RenderPass& render_pass);
};

}  // namespace rd

}  // namespace luka
