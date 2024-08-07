// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/gpu/gpu.h"
#include "rendering/framework/spirv.h"
#include "resource/asset/asset.h"

namespace luka::fw {

constexpr u32 kComputeImageInfoMaxCount{20};

class ComputeJob {
 public:
  ComputeJob() = default;

  ComputeJob(
      std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset, u32 frame_count,
      const std::vector<std::vector<vk::raii::ImageView>>&
          attachment_image_views,
      const ast::ComputeJob& ast_compute_job,
      std::vector<std::unordered_map<std::string, vk::Image>>& shared_images,
      std::vector<std::unordered_map<std::string, vk::ImageView>>&
          shared_image_views);

  const vk::raii::Pipeline* GetPipeline() const;
  const vk::raii::PipelineLayout* GetPipelineLayout() const;
  const vk::raii::DescriptorSets& GetDescriptorSets(u32 frame_index) const;
  u32 GetGroupCountX() const;
  u32 GetGroupCountY() const;
  u32 GetGroupCountZ() const;

  void PreTransferResources(const vk::raii::CommandBuffer& command_buffer,
                            u32 frame_index) const;
  void PostTransferResources(const vk::raii::CommandBuffer& command_buffer,
                             u32 frame_index) const;

 private:
  void ParseShaderResources(
      const SPIRV*& spirv,
      std::unordered_map<std::string, ShaderResource>& name_shader_resources,
      std::unordered_map<u32, std::vector<ShaderResource>>&
          set_shader_resources,
      std::vector<u32>& sorted_sets,
      std::vector<vk::PushConstantRange>& push_constant_ranges);

  void CreatePipelineResources(
      const std::unordered_map<u32, std::vector<ShaderResource>>&
          set_shader_resources,
      const std::vector<u32>& sorted_sets,
      const std::vector<vk::PushConstantRange>& push_constant_ranges);

  void CreatePipeline(const SPIRV* spirv);

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
      const vk::ComputePipelineCreateInfo& compute_pipeline_ci, u64 hash_value,
      const std::string& name = {}, i32 index = -1);

  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;

  u32 frame_count_{};
  const std::vector<std::vector<vk::raii::ImageView>>*
      attachment_image_views_{};
  const ast::ComputeJob* ast_compute_job_{};
  std::vector<std::unordered_map<std::string, vk::Image>>* shared_images_{};
  std::vector<std::unordered_map<std::string, vk::ImageView>>*
      shared_image_views_{};

  u32 shader_{};

  bool need_resize_{};

  bool has_descriptor_set_{};
  const vk::raii::PipelineLayout* pipeline_layout_{};
  std::vector<vk::raii::DescriptorSets> descriptor_sets_;
  bool has_push_constant_{};
  const vk::raii::Pipeline* pipeline_{};

  u32 group_count_x_{40};
  u32 group_count_y_{40};
  u32 group_count_z_{40};

  u32 descriptor_set_index_{UINT32_MAX};

  std::unordered_map<u64, SPIRV> spirv_shaders_;
  std::unordered_map<u64, vk::raii::DescriptorSetLayout>
      descriptor_set_layouts_;
  std::unordered_map<u64, vk::raii::PipelineLayout> pipeline_layouts_;
  std::unordered_map<u64, vk::raii::ShaderModule> shader_modules_;
  std::unordered_map<u64, vk::raii::Pipeline> pipelines_;

  std::vector<std::map<vk::Image, std::pair<vk::ImageLayout, vk::ImageLayout>>>
      image_layout_trans_;
};

}  // namespace luka::fw