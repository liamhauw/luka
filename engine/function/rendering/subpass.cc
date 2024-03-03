// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

#include <vulkan/vulkan_hash.hpp>

namespace luka {

namespace rd {

Subpass::Subpass(std::shared_ptr<Gpu> gpu,
                 const vk::raii::RenderPass& render_pass, u32 frame_count)
    : gpu_{gpu}, render_pass_{*render_pass}, frame_count_{frame_count} {}

vk::DescriptorSetLayout Subpass::GetBindlessDescriptorSetLayout() {
  return bindless_descriptor_set_layout_;
}

const vk::raii::DescriptorSet& Subpass::GetBindlessDescriptorSet() {
  return bindless_descriptor_sets_.front();
}

std::vector<DrawElement>& Subpass::GetDrawElements() { return draw_elements_; }

const vk::raii::DescriptorSetLayout& Subpass::RequestDescriptorSetLayout(
    const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
    const std::string& name) {
  u64 hash_value{0};
  HashCombine(hash_value, descriptor_set_layout_ci.flags);
  for (u32 i{0}; i < descriptor_set_layout_ci.bindingCount; ++i) {
    HashCombine(hash_value, descriptor_set_layout_ci.pBindings[i]);
  }

  auto it{descriptor_set_layouts_.find(hash_value)};
  if (it != descriptor_set_layouts_.end()) {
    return it->second;
  }

  vk::raii::DescriptorSetLayout descriptor_set_layout{
      gpu_->CreateDescriptorSetLayout(descriptor_set_layout_ci, name)};

  auto it1{descriptor_set_layouts_.emplace(hash_value,
                                           std::move(descriptor_set_layout))};

  return it1.first->second;
}

const vk::raii::PipelineLayout& Subpass::RequestPipelineLayout(
    const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
    const std::string& name) {
  u64 hash_value{0};
  HashCombine(hash_value, pipeline_layout_ci.flags);
  for (u32 i{0}; i < pipeline_layout_ci.setLayoutCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pSetLayouts[i]);
  }
  for (u32 i{0}; i < pipeline_layout_ci.pushConstantRangeCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pPushConstantRanges[i]);
  }

  auto it{pipeline_layouts_.find(hash_value)};
  if (it != pipeline_layouts_.end()) {
    return it->second;
  }

  vk::raii::PipelineLayout pipeline_layout{
      gpu_->CreatePipelineLayout(pipeline_layout_ci, name)};

  auto it1{pipeline_layouts_.emplace(hash_value, std::move(pipeline_layout))};

  return it1.first->second;
}

const vk::raii::ShaderModule& Subpass::RequestShaderModule(
    const vk::ShaderModuleCreateInfo& shader_module_ci,
    const std::string& name) {
  u64 hash_value{0};
  HashCombine(hash_value, shader_module_ci);

  auto it{shader_modules_.find(hash_value)};
  if (it != shader_modules_.end()) {
    return it->second;
  }

  vk::raii::ShaderModule shader_module{
      gpu_->CreateShaderModule(shader_module_ci, name)};

  auto it1{shader_modules_.emplace(hash_value, std::move(shader_module))};

  return it1.first->second;
}

const vk::raii::Pipeline& Subpass::RequestPipeline(
    const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci,
    const std::string& name) {
  u64 hash_value{0};
  for (u32 i{0}; i < graphics_pipeline_ci.stageCount; ++i) {
    HashCombine(hash_value, graphics_pipeline_ci.pStages[i]);
  }

  auto it{pipelines_.find(hash_value)};
  if (it != pipelines_.end()) {
    return it->second;
  }

  vk::raii::Pipeline pipeline{gpu_->CreatePipeline(graphics_pipeline_ci, name)};

  auto it1{pipelines_.emplace(hash_value, std::move(pipeline))};

  return it1.first->second;
}

}  // namespace rd

}  // namespace luka
