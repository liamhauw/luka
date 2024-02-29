// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/math.h"
#include "function/rendering/spirv.h"
#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

class GeometrySubpass : public Subpass {
 public:
  GeometrySubpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                  std::shared_ptr<SceneGraph> scene_graph,
                  const vk::raii::RenderPass& render_pass, u32 frame_count);
  ~GeometrySubpass() = default;
  
 private:
  void CreateDrawElements() override;

  DrawElement CreateDrawElement(const glm::mat4& model_matrix,
                                const sg::Primitive& primitive);
};

}  // namespace rd

}  // namespace luka
