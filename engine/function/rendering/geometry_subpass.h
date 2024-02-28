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

struct alignas(16) GlobalUniform {
  glm::mat4 model;
  glm::mat4 view_projection;
  glm::vec3 camera_position;
};

struct PBRMaterialUniform {
  glm::vec4 base_color_factor;
  float metallic_factor;
  float roughness_factor;
};

class GeometrySubpass : public Subpass {
 public:
  GeometrySubpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                  std::shared_ptr<SceneGraph> scene_graph,
                  const vk::raii::RenderPass& render_pass);
  ~GeometrySubpass() = default;

 private:
  void CreateDrawElements(const vk::raii::RenderPass& render_pass) override;

  DrawElement CreateDrawElement(const sg::Primitive& primitive,
                                const vk::raii::RenderPass& render_pass);

  bool global_created_{false};
  GlobalUniform global_uniform_;
  gpu::Buffer global_uniform_buffer_;
};

}  // namespace rd

}  // namespace luka
