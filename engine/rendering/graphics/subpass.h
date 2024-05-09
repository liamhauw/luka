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

struct SubpassUniform {
  glm::mat4 pv;
  glm::mat4 inverse_pv;
  glm::vec4 camera_position;
  ast::PunctualLight punctual_lights[ast::gMaxPunctualLightCount];
};

struct DrawElementUniform {
  glm::mat4 m;
  glm::mat4 inverse_m;
  glm::vec4 base_color_factor;
  glm::uvec4 sampler_indices;
  glm::uvec4 image_indices;
  f32 metallic_factor;
  f32 roughness_factor;
  u32 alpha_model;
  float alpha_cutoff;
};

struct DrawElmentVertexInfo {
  u32 location{UINT32_MAX};
  std::vector<vk::Buffer> buffers;
  std::vector<u64> offsets;
};

struct DrawElement {
  bool has_scene{false};
  bool has_descriptor_set{false};
  const vk::raii::PipelineLayout* pipeline_layout{nullptr};
  std::vector<vk::raii::DescriptorSets> descriptor_sets;
  std::vector<DrawElementUniform> uniforms;
  std::vector<gpu::Buffer> uniform_buffers;
  u64 vertex_count{UINT64_MAX};
  std::vector<DrawElmentVertexInfo> vertex_infos;
  bool has_index{false};
  const ast::sc::IndexAttribute* index_attribute{nullptr};
  const vk::raii::Pipeline* pipeline{nullptr};
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

  void Resize(const std::vector<std::vector<vk::raii::ImageView>>&
                  attachment_image_views);

  void Update(u32 frame_index);

  const std::string& GetName() const;

  const std::vector<DrawElement>& GetDrawElements() const;

  bool HasPushConstant() const;
  void PushConstants(const vk::raii::CommandBuffer& command_buffer,
                     vk::PipelineLayout pipeline_layout) const;

  bool HasSubpassDescriptorSet() const;
  u32 GetSubpassDescriptorSetIndex() const;
  const vk::raii::DescriptorSet& GetSubpassDescriptorSet(u32 frame_index) const;

  bool HasBindlessDescriptorSet() const;
  u32 GetBindlessDescriptorSetIndex() const;
  const vk::raii::DescriptorSet& GetBindlessDescriptorSet() const;

  u32 GetDrawElementDescriptorSetIndex() const;

 protected:
  void CreateDrawElements();

  DrawElement CreateDrawElement(const glm::mat4& model_matrix = {},
                                const glm::mat4& inverse_model_matrix = {},
                                const ast::sc::Primitive& primitive = {},
                                u32 primitive_index = -1);

  void ParseShaderResources(
      const ast::sc::Primitive& primitive, std::vector<const SPIRV*>& spirvs,
      std::unordered_map<std::string, ShaderResource>& name_shader_resources,
      std::unordered_map<u32, std::vector<ShaderResource>>&
          set_shader_resources,
      std::vector<u32>& sorted_sets,
      std::vector<vk::PushConstantRange>& push_constant_ranges);

  void CreatePipelineResources(
      const glm::mat4& model_matrix, const glm::mat4& inverse_model_matrix,
      const ast::sc::Primitive& primitive, u32 primitive_index,
      const std::unordered_map<std::string, ShaderResource>&
          name_shader_resources,
      const std::unordered_map<u32, std::vector<ShaderResource>>&
          set_shader_resources,
      const std::vector<u32>& sorted_sets,
      const std::vector<vk::PushConstantRange>& push_constant_ranges,
      DrawElement& draw_element);

  void CreatePipeline(const ast::sc::Primitive& primitive,
                      const std::vector<const SPIRV*>& spirvs,
                      const std::unordered_map<std::string, ShaderResource>&
                          name_shader_resources,
                      DrawElement& draw_element);

  const SPIRV& RequestSpirv(const ast::Shader& shader,
                            const std::vector<std::string>& processes,
                            vk::ShaderStageFlagBits shader_stage);

  const vk::raii::DescriptorSetLayout& RequestDescriptorSetLayout(
      const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
      const std::string& name = {}, i32 index = -1);

  const vk::raii::PipelineLayout& RequestPipelineLayout(
      const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
      const std::string& name = {}, i32 index = -1);

  const vk::raii::ShaderModule& RequestShaderModule(
      const vk::ShaderModuleCreateInfo& shader_module_ci, u64 hash_value,
      const std::string& name = {}, i32 index = -1);

  const vk::raii::Pipeline& RequestPipeline(
      const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci,
      u64 hash_value, const std::string& name = {}, i32 index = -1);

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
  const std::vector<u32>* lights_;
  const std::unordered_map<vk::ShaderStageFlagBits, u32>* shaders_;
  bool has_scene_;
  bool has_light_;
  std::vector<SubpassUniform> subpass_uniforms_;
  std::vector<gpu::Buffer> subpass_uniform_buffers_;

  bool need_resize_{false};

  std::vector<ast::PunctualLight> punctual_lights_;

  bool has_subpass_descriptor_set_{false};
  u32 subpass_descriptor_set_index_{UINT32_MAX};
  const vk::raii::DescriptorSetLayout* subpass_descriptor_set_layout_{nullptr};
  vk::raii::DescriptorSets subpass_descriptor_sets_{nullptr};
  bool subpass_desciptor_set_updated_{false};

  bool has_bindless_descriptor_set_{false};
  u32 bindless_descriptor_set_index_{UINT32_MAX};
  const vk::raii::DescriptorSetLayout* bindless_descriptor_set_layout_{nullptr};
  vk::raii::DescriptorSet bindless_descriptor_set_{nullptr};
  const std::vector<std::string> kWantedTextures{
      "base_color_texture", "metallic_roughness_texture", "normal_texture"};
  const u32 kBindlessSamplerMaxCount{8};
  const u32 kBindlessImageMaxCount{80};
  u32 bindless_sampler_index_{0};
  u32 bindless_image_index_{0};

  u32 draw_element_descriptor_set_index_{UINT32_MAX};

  bool has_push_constant_{false};

  std::unordered_map<u64, SPIRV> spirv_shaders_;
  std::unordered_map<u64, vk::raii::DescriptorSetLayout>
      descriptor_set_layouts_;
  std::unordered_map<u64, vk::raii::PipelineLayout> pipeline_layouts_;
  std::unordered_map<u64, vk::raii::ShaderModule> shader_modules_;
  std::unordered_map<u64, vk::raii::Pipeline> pipelines_;
  std::unordered_map<u64, u32> sampler_indices_;
  std::unordered_map<u64, u32> image_indices_;

  std::vector<DrawElement> draw_elements_;
};

}  // namespace gs

}  // namespace luka
