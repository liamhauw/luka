// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

#undef MemoryBarrier
#include <vulkan/vulkan_hash.hpp>

#include "core/log.h"
#include "core/util.h"
#include "resource/config/generated/root_path.h"

namespace luka {

namespace rd {

Subpass::Subpass(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu,
                 std::shared_ptr<Asset> asset, std::shared_ptr<Camera> camera,
                 const vk::raii::RenderPass& render_pass, u32 frame_count)
    : config_{config},
      gpu_{gpu},
      asset_{asset},
      camera_{camera},
      render_pass_{*render_pass},
      frame_count_{frame_count} {}

vk::DescriptorSetLayout Subpass::GetBindlessDescriptorSetLayout() {
  return bindless_descriptor_set_layout_;
}

const vk::raii::DescriptorSet& Subpass::GetBindlessDescriptorSet() {
  return bindless_descriptor_sets_.front();
}

std::vector<DrawElement>& Subpass::GetDrawElements() { return draw_elements_; }

const SPIRV& Subpass::RequesetSpirv(const ast::Shader& shader,
                                    const std::vector<std::string>& processes,
                                    vk::ShaderStageFlagBits shader_stage,
                                    const std::string& name) {
  u64 hash_value{shader.GetHashValue(processes)};

  auto it{spirv_shaders_.find(hash_value)};
  if (it != spirv_shaders_.end()) {
    return it->second;
  }

  SPIRV spirv{shader, processes, shader_stage, hash_value};

  auto it1{spirv_shaders_.emplace(hash_value, std::move(spirv))};

  return it1.first->second;
}

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
    const vk::ShaderModuleCreateInfo& shader_module_ci, u64 hash_value,
    const std::string& name) {
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
    const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci, u64 hash_value,
    const std::string& name) {
  auto it{pipelines_.find(hash_value)};
  if (it != pipelines_.end()) {
    return it->second;
  }

  vk::PipelineCacheCreateInfo pipeline_cache_ci;

  std::filesystem::path root_path{GetPath(LUKA_ROOT_PATH)};
  std::filesystem::path cache_path{root_path / "cache"};
  std::filesystem::path pipeline_cache_file{
      cache_path / (std::to_string(hash_value) + ".pipeline_cache")};
  std::vector<u8> pipeline_cache_data;

  bool has_cache{false};
  if (std::filesystem::exists(pipeline_cache_file)) {
    pipeline_cache_data = LoadBinary(pipeline_cache_file);

    vk::PipelineCacheHeaderVersionOne* header_version_one{
        reinterpret_cast<vk::PipelineCacheHeaderVersionOne*>(
            pipeline_cache_data.data())};
    vk::PhysicalDeviceProperties physical_device_properties{
        gpu_->GetPhysicalDeviceProperties()};

    if (header_version_one->headerSize > 0 &&
        header_version_one->headerVersion ==
            vk::PipelineCacheHeaderVersion::eOne &&
        header_version_one->vendorID == physical_device_properties.vendorID &&
        header_version_one->deviceID == physical_device_properties.deviceID) {
      has_cache = true;

      pipeline_cache_ci.initialDataSize = pipeline_cache_data.size();
      pipeline_cache_ci.pInitialData = pipeline_cache_data.data();
    }
  }

  vk::raii::PipelineCache pipeline_cache{
      gpu_->CreatePipelineCache(pipeline_cache_ci)};

  vk::raii::Pipeline pipeline{
      gpu_->CreatePipeline(graphics_pipeline_ci, pipeline_cache, name)};

  if (!has_cache) {
    std::vector<u8> pipeline_cache_data{pipeline_cache.getData()};
    if (!std::filesystem::exists(cache_path)) {
      std::filesystem::create_directories(cache_path);
    }
    SaveBinary(pipeline_cache_data, pipeline_cache_file);
  }

  auto it1{pipelines_.emplace(hash_value, std::move(pipeline))};

  return it1.first->second;
}

}  // namespace rd

}  // namespace luka
