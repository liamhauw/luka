// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/framework/compute_job.h"

#include "core/log.h"

namespace luka::fw {
ComputeJob::ComputeJob(
    std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset, u32 frame_count,
    const std::vector<std::vector<vk::raii::ImageView>>& attachment_image_views,
    const ast::ComputeJob& ast_compute_job,
    std::vector<std::unordered_map<std::string, vk::ImageView>>&
        shared_image_views)
    : gpu_{std::move(gpu)},
      asset_{std::move(asset)},
      frame_count_{frame_count},
      attachment_image_views_{&attachment_image_views},
      ast_compute_job_{&ast_compute_job},
      shared_image_views_{&shared_image_views},
      shader_{ast_compute_job_->shader} {
  const SPIRV* spirv{};
  std::unordered_map<std::string, ShaderResource> name_shader_resources;
  std::unordered_map<u32, std::vector<ShaderResource>> set_shader_resources;
  std::vector<u32> sorted_sets;
  std::vector<vk::PushConstantRange> push_constant_ranges;

  ParseShaderResources(spirv, name_shader_resources, set_shader_resources,
                       sorted_sets, push_constant_ranges);
  CreatePipelineResources(name_shader_resources, set_shader_resources,
                          sorted_sets, push_constant_ranges);
  CreatePipeline(spirv, name_shader_resources);
}

void ComputeJob::ParseShaderResources(
    const SPIRV* spirv,
    std::unordered_map<std::string, ShaderResource>& name_shader_resources,
    std::unordered_map<u32, std::vector<ShaderResource>>& set_shader_resources,
    std::vector<u32>& sorted_sets,
    std::vector<vk::PushConstantRange>& push_constant_ranges) {
  const SPIRV& comp_spirv{RequestSpirv(asset_->GetShader(shader_), {},
                                       vk::ShaderStageFlagBits::eCompute)};

  spirv = &comp_spirv;
  const auto& shader_resources{spirv->GetShaderResources()};
  for (const auto& shader_resource : shader_resources) {
    const std::string& name{shader_resource.name};

    auto it{name_shader_resources.find(name)};
    if (it != name_shader_resources.end()) {
      it->second.stage |= shader_resource.stage;
    } else {
      name_shader_resources.emplace(name, shader_resource);
    }
  }

  for (const auto& name_shader_resource : name_shader_resources) {
    const auto& shader_resource{name_shader_resource.second};

    if (shader_resource.type == ShaderResourceType::kSampler ||
        shader_resource.type == ShaderResourceType::kCombinedImageSampler ||
        shader_resource.type == ShaderResourceType::kSampledImage ||
        shader_resource.type == ShaderResourceType::kStorageImage ||
        shader_resource.type == ShaderResourceType::kUniformBuffer) {
      auto it{set_shader_resources.find(shader_resource.set)};
      if (it != set_shader_resources.end()) {
        it->second.push_back(shader_resource);
      } else {
        set_shader_resources.emplace(
            shader_resource.set, std::vector<ShaderResource>{shader_resource});
        sorted_sets.push_back(shader_resource.set);
      }
    } else if (shader_resource.type ==
               ShaderResourceType::kPushConstantBuffer) {
      push_constant_ranges.emplace_back(
          shader_resource.stage, shader_resource.offset, shader_resource.size);
    }
  }

  std::sort(sorted_sets.begin(), sorted_sets.end());
  if (!sorted_sets.empty() && (sorted_sets.back() != sorted_sets.size() - 1)) {
    THROW("Descriptor sets is not continuous.");
  }
}

void ComputeJob::CreatePipelineResources(
    const std::unordered_map<std::string, ShaderResource>&
        name_shader_resources,
    const std::unordered_map<u32, std::vector<ShaderResource>>&
        set_shader_resources,
    const std::vector<u32>& sorted_sets,
    const std::vector<vk::PushConstantRange>& push_constant_ranges) {}

void ComputeJob::CreatePipeline(
    const SPIRV* spirv, const std::unordered_map<std::string, ShaderResource>&
                            name_shader_resources) {}

const SPIRV& ComputeJob::RequestSpirv(const ast::Shader& shader,
                                      const std::vector<std::string>& processes,
                                      vk::ShaderStageFlagBits shader_stage) {
  u64 hash_value{shader.GetHashValue(processes)};

  auto it{spirv_shaders_.find(hash_value)};
  if (it != spirv_shaders_.end()) {
    return it->second;
  }

  std::filesystem::path root_path{GetPath(LUKA_ROOT_PATH)};
  std::filesystem::path cache_path{root_path / ".cache" / "engine"};
  std::filesystem::path spirv_cache_file{
      cache_path / ("spirv_" + std::to_string(hash_value) + ".cache")};

  std::vector<u32> spirv_cache_data;
  if (std::filesystem::exists(spirv_cache_file)) {
    spirv_cache_data = LoadBinaryU32(spirv_cache_file);
  } else {
    spirv_cache_data = shader.CompileToSpirv(processes);
    if (!std::filesystem::exists(cache_path)) {
      std::filesystem::create_directories(cache_path);
    }
    SaveBinaryU32(spirv_cache_data, spirv_cache_file);
  }

  SPIRV spirv{spirv_cache_data, shader_stage, hash_value};

  auto it1{spirv_shaders_.emplace(hash_value, std::move(spirv))};

  return it1.first->second;
}

}  // namespace luka::fw