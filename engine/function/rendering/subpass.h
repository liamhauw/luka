// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"
#include "function/gpu/gpu.h"
#include "function/rendering/spirv.h"
#include "function/scene_graph/scene_graph.h"
#include "resource/asset/asset.h"

namespace luka {

namespace rd {

struct alignas(16) DrawElementUniform {
  glm::mat4 m;
  glm::vec4 base_color_factor;
  glm::uvec4 image_indices;
};

struct DrawElement {
  DrawElement() = default;

  DrawElement(DrawElement&& rhs) noexcept
      : pipeline(std::move(rhs.pipeline)),
        pipeline_layout(std::move(rhs.pipeline_layout)),
        location_vertex_attributes(std::move(rhs.location_vertex_attributes)),
        vertex_count(rhs.vertex_count),
        index_attribute(rhs.index_attribute),
        has_index(rhs.has_index),
        draw_element_uniforms{std::move(rhs.draw_element_uniforms)},
        draw_element_uniform_buffers{
            std::move(rhs.draw_element_uniform_buffers)},
        descriptor_sets(std::move(rhs.descriptor_sets)) {}

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
  Subpass(std::shared_ptr<Asset> asset, std::shared_ptr<Camera> camera,
          std::shared_ptr<Gpu> gpu, std::shared_ptr<SceneGraph> scene_graph,
          const vk::raii::RenderPass& render_pass, u32 frame_count);
  virtual ~Subpass() = default;

  virtual void PushConstants(const vk::raii::CommandBuffer& command_buffer,
                             vk::PipelineLayout pipeline_layout) = 0;

  vk::DescriptorSetLayout GetBindlessDescriptorSetLayout();
  const vk::raii::DescriptorSet& GetBindlessDescriptorSet();
  std::vector<DrawElement>& GetDrawElements();

 protected:
  virtual void CreateBindlessDescriptorSets() = 0;
  virtual void CreateDrawElements() = 0;

  const vk::raii::DescriptorSetLayout& RequestDescriptorSetLayout(
      const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
      const std::string& name = {});

  const vk::raii::PipelineLayout& RequestPipelineLayout(
      const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
      const std::string& name = {});

  const vk::raii::ShaderModule& RequestShaderModule(
      const vk::ShaderModuleCreateInfo& shader_module_ci, u64 hash_value,
      const std::string& name = {});

  const vk::raii::Pipeline& RequestPipeline(
      const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci,
      u64 hash_value, const std::string& name = {});

  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<SceneGraph> scene_graph_;
  vk::RenderPass render_pass_;
  u32 frame_count_;

  vk::DescriptorSetLayout bindless_descriptor_set_layout_{nullptr};
  vk::raii::DescriptorSets bindless_descriptor_sets_{nullptr};

  std::vector<DrawElement> draw_elements_;

  std::unordered_map<u64, SPIRV> spirv_shaders_;
  std::unordered_map<u64, vk::raii::DescriptorSetLayout>
      descriptor_set_layouts_;
  std::unordered_map<u64, vk::raii::PipelineLayout> pipeline_layouts_;
  std::unordered_map<u64, vk::raii::ShaderModule> shader_modules_;
  std::unordered_map<u64, vk::raii::Pipeline> pipelines_;
  u32 global_image_index_{0};
};

}  // namespace rd

}  // namespace luka
