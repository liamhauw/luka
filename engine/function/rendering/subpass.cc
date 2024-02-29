// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

Subpass::Subpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
                 std::shared_ptr<SceneGraph> scene_graph,
                 const vk::raii::RenderPass& render_pass, u32 frame_count)
    : asset_{asset},
      gpu_{gpu},
      scene_graph_{scene_graph},
      render_pass_{*render_pass},
      frame_count_{frame_count},
      global_uniforms_(frame_count) {
  for (u32 i{0}; i < frame_count_; ++i) {
    vk::BufferCreateInfo uniform_buffer_ci{
        {},
        sizeof(GlobalUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive};
    global_uniform_buffers_.push_back(gpu_->CreateBuffer(
        uniform_buffer_ci, &(global_uniforms_[i]), true, "globale_uniform"));
  }
}

void Subpass::Update(u32 active_frame_index, std::shared_ptr<Camera> camera) {
  auto& global_uniform{global_uniforms_[active_frame_index]};

  const glm::mat4& view{camera->GetViewMatrix()};
  const glm::mat4& projection{camera->GetProjectionMatrix()};
  const glm::vec3& camera_position{camera->GetPosition()};

  global_uniform.pv = projection * view;
  global_uniform.camera_position = camera_position;

  gpu::Buffer& uniform_buffer{global_uniform_buffers_[active_frame_index]};

  void* mapped_data{uniform_buffer.Map()};
  memcpy(mapped_data, &global_uniform, sizeof(global_uniform));
}

const std::vector<DrawElement>& Subpass::GetDrawElements() const {
  return draw_elements_;
}

}  // namespace rd

}  // namespace luka
