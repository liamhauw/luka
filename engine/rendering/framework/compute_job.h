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
class ComputeJob {
 public:
  ComputeJob() = default;

  ComputeJob(std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
             u32 frame_count,
             const std::vector<std::vector<vk::raii::ImageView>>&
                 attachment_image_views,
             const ast::ComputeJob& ast_compute_job,
             std::vector<std::unordered_map<std::string, vk::ImageView>>&
                 shared_image_views);

 private:
  void ParseShaderResources(
      const SPIRV* spirv,
      std::unordered_map<std::string, ShaderResource>& name_shader_resources,
      std::unordered_map<u32, std::vector<ShaderResource>>&
          set_shader_resources,
      std::vector<u32>& sorted_sets,
      std::vector<vk::PushConstantRange>& push_constant_ranges);

  void CreatePipelineResources(
      const std::unordered_map<std::string, ShaderResource>&
          name_shader_resources,
      const std::unordered_map<u32, std::vector<ShaderResource>>&
          set_shader_resources,
      const std::vector<u32>& sorted_sets,
      const std::vector<vk::PushConstantRange>& push_constant_ranges);

  void CreatePipeline(const SPIRV* spirv,
                      const std::unordered_map<std::string, ShaderResource>&
                          name_shader_resources);

  const SPIRV& RequestSpirv(const ast::Shader& shader,
                            const std::vector<std::string>& processes,
                            vk::ShaderStageFlagBits shader_stage);

  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;

  u32 frame_count_{};
  const std::vector<std::vector<vk::raii::ImageView>>*
      attachment_image_views_{};
  const ast::ComputeJob* ast_compute_job_;
  std::vector<std::unordered_map<std::string, vk::ImageView>>*
      shared_image_views_;

  u32 shader_{};

  vk::raii::Pipeline pipeline_{nullptr};
  vk::raii::DescriptorSets descriptor_sets_{nullptr};
  u32 group_count_x_{};
  u32 group_count_y_{};
  u32 group_count_z_{};

  std::unordered_map<u64, SPIRV> spirv_shaders_;
};

}  // namespace luka::fw