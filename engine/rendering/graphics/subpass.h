// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"
#include "rendering/graphics/spirv.h"
#include "resource/asset/asset.h"
#include "resource/gpu/gpu.h"

namespace luka {

namespace gs {

struct PushConstantUniform {
  glm::mat4 pv;
  glm::vec3 camera_position;
};

struct DrawElmentVertexInfo {
  u32 location;
  std::vector<vk::Buffer> buffers;
  std::vector<u64> offsets;
};

struct alignas(16) DrawElementUniform {
  glm::mat4 m;
  glm::vec4 base_color_factor;
  glm::uvec4 sampler_indices;
  glm::uvec4 image_indices;
};

struct DrawElement {
  DrawElement() = default;

  DrawElement(DrawElement&& rhs) noexcept
      : has_primitive{rhs.has_primitive},
        pipeline_layout{std::move(rhs.pipeline_layout)},
        pipeline(std::move(rhs.pipeline)),
        vertex_infos(std::move(rhs.vertex_infos)),
        vertex_count(rhs.vertex_count),
        index_attribute(rhs.index_attribute),
        has_index(rhs.has_index),
        uniforms{std::move(rhs.uniforms)},
        uniform_buffers{
            std::move(rhs.uniform_buffers)},
        descriptor_sets(std::move(rhs.descriptor_sets)) {}

  bool has_primitive;
  const vk::raii::PipelineLayout* pipeline_layout;
  std::vector<DrawElmentVertexInfo> vertex_infos;  
  u64 vertex_count;
  bool has_index;
  const ast::sc::IndexAttribute* index_attribute;
  const vk::raii::Pipeline* pipeline;

  // For every frame.
  std::vector<DrawElementUniform> uniforms;
  std::vector<gpu::Buffer> uniform_buffers;
  std::vector<vk::raii::DescriptorSets> descriptor_sets;
};

class Subpass {
 public:
  Subpass(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
          std::shared_ptr<Camera> camera, u32 frame_count,
          vk::RenderPass render_pass,
          const std::vector<std::vector<vk::raii::ImageView>>&
              attachment_image_views,
          u32 color_attachment_count,
          const std::vector<ast::Subpass>& ast_subpasses, u32 subpass_index,
          std::vector<std::unordered_map<std::string, vk::ImageView>>&
              shared_image_views);

  void PushConstants(const vk::raii::CommandBuffer& command_buffer,
                     vk::PipelineLayout pipeline_layout) const;

  const std::string& GetName() const;
  vk::DescriptorSetLayout GetBindlessDescriptorSetLayout() const;
  const vk::raii::DescriptorSet& GetBindlessDescriptorSet() const;
  bool HasPushConstant() const;
  const std::vector<DrawElement>& GetDrawElements() const;

  void Resize(const std::vector<std::vector<vk::raii::ImageView>>&
                  attachment_image_views);

 protected:
  void CreateBindlessDescriptorSets();
  void CreateDrawElements();

  DrawElement CreateDrawElement(const glm::mat4& model_matrix = {},
                                const ast::sc::Primitive& primitive = {});

  const SPIRV& RequesetSpirv(const ast::Shader& shader,
                             const std::vector<std::string>& processes,
                             vk::ShaderStageFlagBits shader_stage,
                             const std::string& name = {});

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

  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Camera> camera_;

  u32 frame_count_;
  vk::RenderPass render_pass_;
  const std::vector<std::vector<vk::raii::ImageView>>* attachment_image_views_;
  u32 color_attachment_count_;
  const std::vector<ast::Subpass>* ast_subpasses_;
  u32 subpass_index_;
  std::vector<std::unordered_map<std::string, vk::ImageView>>*
      shared_image_views_;

  const ast::Subpass* ast_subpass_;
  std::string name_;
  const std::vector<u32>* scenes_;
  const std::unordered_map<vk::ShaderStageFlagBits, u32>* shaders_;
  bool has_primitive_;

  bool has_bindless_descriptor_set_{false};
  vk::DescriptorSetLayout bindless_descriptor_set_layout_{nullptr};
  vk::raii::DescriptorSets bindless_descriptor_sets_{nullptr};

  bool has_push_constant_{false};

  std::vector<DrawElement> draw_elements_;

  std::unordered_map<u64, SPIRV> spirv_shaders_;
  std::unordered_map<u64, vk::raii::DescriptorSetLayout>
      descriptor_set_layouts_;
  std::unordered_map<u64, vk::raii::PipelineLayout> pipeline_layouts_;
  std::unordered_map<u64, vk::raii::ShaderModule> shader_modules_;
  std::unordered_map<u64, vk::raii::Pipeline> pipelines_;
  std::unordered_map<u64, u32> sampler_indices_;
  std::unordered_map<u64, u32> image_indices_;

  u32 global_sampler_index_{0};
  u32 global_image_index_{0};

  bool need_resize_{false};
};

}  // namespace gs

}  // namespace luka
