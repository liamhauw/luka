// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/spirv.h"
#include "function/scene_graph/scene_graph.h"
#include "resource/asset/asset.h"
#include "function/camera/camera.h"

namespace luka {

namespace rd {

struct alignas(16) GlobalUniform {
  glm::mat4 pv;
  glm::vec3 camera_position;
};

struct alignas(16) DrawElementUniform {
  glm::mat4 m;
  glm::vec4 base_color_factor;
};

struct DrawElement {
  DrawElement() = default;

  DrawElement(DrawElement&& other) noexcept
      : pipeline(std::move(other.pipeline)),
        pipeline_layout(std::move(other.pipeline_layout)),
        location_vertex_attributes(std::move(other.location_vertex_attributes)),
        vertex_count(other.vertex_count),
        index_attribute(other.index_attribute),
        has_index(other.has_index),
        draw_element_uniforms{std::move(other.draw_element_uniforms)},
        draw_element_uniform_buffers{
            std::move(other.draw_element_uniform_buffers)},
        descriptor_sets(std::move(other.descriptor_sets)) {}

  vk::Pipeline pipeline;
  vk::PipelineLayout pipeline_layout;
  std::map<u32, const sg::VertexAttribute*> location_vertex_attributes;
  u64 vertex_count;
  const sg::IndexAttribute* index_attribute;
  bool has_index;

  // For every frame.
  std::vector<DrawElementUniform> draw_element_uniforms;
  std::vector<gpu::Buffer> draw_element_uniform_buffers;
  std::vector<vk::raii::DescriptorSets> descriptor_sets;
};

class Subpass {
 public:
  Subpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
          std::shared_ptr<SceneGraph> scene_graph,
          const vk::raii::RenderPass& render_pass, u32 frame_count);
  virtual ~Subpass() = default;

  void Update(u32 active_frame_index, std::shared_ptr<Camera> camera);

  const std::vector<DrawElement>& GetDrawElements() const;

 protected:
  virtual void CreateDrawElements() = 0;

  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<SceneGraph> scene_graph_;

  vk::RenderPass render_pass_;
  u32 frame_count_;

  std::vector<GlobalUniform> global_uniforms_;
  std::vector<gpu::Buffer> global_uniform_buffers_;

  std::map<u64, SPIRV> spirv_shaders_;
  std::vector<DrawElement> draw_elements_;
};

}  // namespace rd

}  // namespace luka
