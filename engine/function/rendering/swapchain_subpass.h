// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/math.h"
#include "function/camera/camera.h"
#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

struct PushConstantUniform {
  glm::mat4 pv;
  glm::vec3 camera_position;
};

class SwapchainSupass : public Subpass {
 public:
  SwapchainSupass(std::shared_ptr<Asset> asset, std::shared_ptr<Camera> camera,
                  std::shared_ptr<Gpu> gpu,
                  std::shared_ptr<SceneGraph> scene_graph,
                  const vk::raii::RenderPass& render_pass, u32 frame_count);
  ~SwapchainSupass() = default;

  void PushConstants(const vk::raii::CommandBuffer& command_buffer,
                     vk::PipelineLayout pipeline_layout) override;

 private:
  void CreateBindlessDescriptorSets() override;
  void CreateDrawElements() override;

  DrawElement CreateDrawElement(const glm::mat4& model_matrix,
                                const ast::sc::Primitive& primitive);

  std::vector<std::string> wanted_textures_{"base_color_texture"};
};

}  // namespace rd

}  // namespace luka
