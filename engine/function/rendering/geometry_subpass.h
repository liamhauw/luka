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

class GeometrySubpass : public Subpass {
 public:
  GeometrySubpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                  std::shared_ptr<SceneGraph> scene_graph,
                  const vk::raii::RenderPass& render_pass, u32 frame_count);
  ~GeometrySubpass() = default;

  void Update(u32 active_frame_index) override;

 private:
  void CreateDrawElements() override;

  DrawElement CreateDrawElement(const sg::Primitive& primitive);

  std::vector<bool> global_created_;
  std::vector<GlobalUniform> global_uniforms_;
  std::vector<gpu::Buffer> global_uniform_buffers_;
};

}  // namespace rd

}  // namespace luka
