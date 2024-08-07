// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/framework/compute_job.h"

#ifdef _MSC_VER
#undef MemoryBarrier
#endif
#include <utility>
#include <vulkan/vulkan_hash.hpp>

#include "core/log.h"

namespace luka::fw {
ComputeJob::ComputeJob(
    std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset, u32 frame_count,
    const std::vector<std::vector<vk::raii::ImageView>>& attachment_image_views,
    const ast::ComputeJob& ast_compute_job,
    std::vector<std::unordered_map<std::string, vk::Image>>& shared_images,
    std::vector<std::unordered_map<std::string, vk::ImageView>>&
        shared_image_views)
    : gpu_{std::move(gpu)},
      asset_{std::move(asset)},
      frame_count_{frame_count},
      attachment_image_views_{&attachment_image_views},
      ast_compute_job_{&ast_compute_job},
      shared_images_{&shared_images},
      shared_image_views_{&shared_image_views},
      shader_{ast_compute_job_->shader} {
  image_layout_trans_.resize(frame_count_);

  const SPIRV* spirv{};
  std::unordered_map<std::string, ShaderResource> name_shader_resources;
  std::unordered_map<u32, std::vector<ShaderResource>> set_shader_resources;
  std::vector<u32> sorted_sets;
  std::vector<vk::PushConstantRange> push_constant_ranges;

  ParseShaderResources(spirv, name_shader_resources, set_shader_resources,
                       sorted_sets, push_constant_ranges);
  CreatePipelineResources(set_shader_resources, sorted_sets,
                          push_constant_ranges);
  CreatePipeline(spirv);
}

const vk::raii::Pipeline* ComputeJob::GetPipeline() const { return pipeline_; }

const vk::raii::PipelineLayout* ComputeJob::GetPipelineLayout() const {
  return pipeline_layout_;
}

const vk::raii::DescriptorSets& ComputeJob::GetDescriptorSets(
    u32 frame_index) const {
  return descriptor_sets_[frame_index];
}

u32 ComputeJob::GetGroupCountX() const { return group_count_x_; }

u32 ComputeJob::GetGroupCountY() const { return group_count_y_; }

u32 ComputeJob::GetGroupCountZ() const { return group_count_z_; }

void ComputeJob::PreTransferResources(
    const vk::raii::CommandBuffer& command_buffer, u32 frame_index) const {
  for (auto tr : image_layout_trans_[frame_index]) {
    vk::Image image{tr.first};
    const auto& layout{tr.second};
    vk::ImageLayout ext{layout.first};
    vk::ImageLayout inn{layout.second};
    vk::ImageMemoryBarrier barrier{
        {},
        vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead,
        ext,
        inn,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image,
        {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
         VK_REMAINING_ARRAY_LAYERS}};
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eAllCommands, {},
                                   {}, {}, barrier);
  }
}

void ComputeJob::PostTransferResources(
    const vk::raii::CommandBuffer& command_buffer, u32 frame_index) const {
  for (auto tr : image_layout_trans_[frame_index]) {
    vk::Image image{tr.first};
    const auto& layout{tr.second};
    vk::ImageLayout ext{layout.first};
    vk::ImageLayout inn{layout.second};
    vk::ImageMemoryBarrier barrier{
        {},
        vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead,
        inn,
        ext,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image,
        {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
         VK_REMAINING_ARRAY_LAYERS}};
    command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eAllCommands, {},
                                   {}, {}, barrier);
  }
}

void ComputeJob::ParseShaderResources(
    const SPIRV*& spirv,
    std::unordered_map<std::string, ShaderResource>& name_shader_resources,
    std::unordered_map<u32, std::vector<ShaderResource>>& set_shader_resources,
    std::vector<u32>& sorted_sets,
    std::vector<vk::PushConstantRange>& push_constant_ranges) {
  spirv = &RequestSpirv(asset_->GetShader(shader_), {},
                        vk::ShaderStageFlagBits::eCompute);

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
    const std::unordered_map<u32, std::vector<ShaderResource>>&
        set_shader_resources,
    const std::vector<u32>& sorted_sets,
    const std::vector<vk::PushConstantRange>& push_constant_ranges) {
  std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
  std::vector<vk::DescriptorImageInfo> image_infos;
  image_infos.reserve(kComputeImageInfoMaxCount);

  std::vector<vk::DescriptorSetLayout> set_layouts;

  for (u32 set : sorted_sets) {
    const vk::raii::DescriptorSetLayout* descriptor_set_layout{};
    const auto& shader_resources{set_shader_resources.at(set)};

    // Create descriptor set layout.
    descriptor_set_index_ = std::min(descriptor_set_index_, set);
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const auto& shader_resource : shader_resources) {
      vk::DescriptorType descriptor_type{};

      if (shader_resource.type == ShaderResourceType::kStorageImage) {
        descriptor_type = vk::DescriptorType::eStorageImage;
      } else {
        THROW("Unsupport descriptor type");
      }

      vk::DescriptorSetLayoutBinding binding{
          shader_resource.binding, descriptor_type, 1, shader_resource.stage};

      bindings.push_back(binding);
    }

    if (bindings.empty()) {
      continue;
    }

    vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_ci{{}, bindings};

    descriptor_set_layout =
        &(RequestDescriptorSetLayout(descriptor_set_layout_ci));

    // Allocate descriptor sets.
    has_descriptor_set_ = true;
    for (u32 i{}; i < frame_count_; ++i) {
      vk::DescriptorSetAllocateInfo descriptor_set_ai{nullptr,
                                                      **descriptor_set_layout};

      vk::raii::DescriptorSets descriptor_sets{
          gpu_->AllocateNormalDescriptorSets(descriptor_set_ai)};

      descriptor_sets_.push_back(std::move(descriptor_sets));
    }

    // Update descriptor sets.
    for (const auto& shader_resource : shader_resources) {
      if (shader_resource.type == ShaderResourceType::kStorageImage) {
        need_resize_ = true;
        for (u32 i{}; i < frame_count_; ++i) {
          vk::Image image{nullptr};

          auto image_it{(*shared_images_)[i].find(shader_resource.name)};
          if (image_it != (*shared_images_)[i].end()) {
            image = image_it->second;
            image_layout_trans_[i].insert(
                {image, std::pair<vk::ImageLayout, vk::ImageLayout>{
                            vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::ImageLayout::eGeneral}});
          } else {
            THROW("Image is nullptr");
          }

          vk::ImageView image_view{nullptr};

          auto it{(*shared_image_views_)[i].find(shader_resource.name)};
          if (it != (*shared_image_views_)[i].end()) {
            image_view = it->second;
          } else {
            THROW("Image view is nullptr");
          }

          vk::DescriptorImageInfo descriptor_image_info{
              *(gpu_->GetSampler()), image_view, vk::ImageLayout::eGeneral};
          image_infos.push_back(descriptor_image_info);

          vk::WriteDescriptorSet write_descriptor_set{
              *(descriptor_sets_[i]
                                [shader_resource.set - descriptor_set_index_]),
              shader_resource.binding, 0, vk::DescriptorType::eStorageImage,
              image_infos.back()};

          write_descriptor_sets.push_back(write_descriptor_set);
        }
      }
    }

    set_layouts.push_back(**descriptor_set_layout);
  }

  if (image_infos.size() >= kComputeImageInfoMaxCount) {
    THROW("The size of sampler_infos ({}) exceeds kSamplerInfoMaxCount ({})",
          image_infos.size(), kComputeImageInfoMaxCount);
  }

  gpu_->UpdateDescriptorSets(write_descriptor_sets);

  // Pipeline layouts.
  vk::PipelineLayoutCreateInfo pipeline_layout_ci;

  if (!set_layouts.empty()) {
    pipeline_layout_ci.setSetLayouts(set_layouts);
  }

  if (!push_constant_ranges.empty()) {
    has_push_constant_ = true;
    pipeline_layout_ci.setPushConstantRanges(push_constant_ranges);
  }

  const vk::raii::PipelineLayout& pipeline_layout{
      RequestPipelineLayout(pipeline_layout_ci)};

  pipeline_layout_ = &pipeline_layout;
}

void ComputeJob::CreatePipeline(const SPIRV* spirv_shader) {
  u64 pipeline_hash_value{};
  u64 shader_module_hash_value{spirv_shader->GetHashValue()};
  HashCombine(pipeline_hash_value, shader_module_hash_value);

  vk::PipelineShaderStageCreateInfo pipeline_shader_stage_ci;
  const std::vector<u32>& spirv{spirv_shader->GetSpirv()};

  vk::ShaderModuleCreateInfo shader_module_ci{
      {}, spirv.size() * 4, spirv.data()};
  const vk::raii::ShaderModule& shader_module{
      RequestShaderModule(shader_module_ci, shader_module_hash_value)};

  vk::PipelineShaderStageCreateInfo shader_stage_ci{
      {}, spirv_shader->GetStage(), *shader_module, "main", nullptr};

  vk::ComputePipelineCreateInfo compute_pipeline_ci{
      {}, shader_stage_ci, **pipeline_layout_};

  pipeline_ = &RequestPipeline(compute_pipeline_ci, pipeline_hash_value);
}

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

const vk::raii::DescriptorSetLayout& ComputeJob::RequestDescriptorSetLayout(
    const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
    const std::string& name, i32 index) {
  u64 hash_value{};
  HashCombine(hash_value, descriptor_set_layout_ci.flags);
  for (u32 i{}; i < descriptor_set_layout_ci.bindingCount; ++i) {
    HashCombine(hash_value, descriptor_set_layout_ci.pBindings[i]);
  }

  auto it{descriptor_set_layouts_.find(hash_value)};
  if (it != descriptor_set_layouts_.end()) {
    return it->second;
  }

  vk::raii::DescriptorSetLayout descriptor_set_layout{
      gpu_->CreateDescriptorSetLayout(descriptor_set_layout_ci, name, index)};

  auto it1{descriptor_set_layouts_.emplace(hash_value,
                                           std::move(descriptor_set_layout))};

  return it1.first->second;
}

const vk::raii::PipelineLayout& ComputeJob::RequestPipelineLayout(
    const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
    const std::string& name, i32 index) {
  u64 hash_value{};
  HashCombine(hash_value, pipeline_layout_ci.flags);
  for (u32 i{}; i < pipeline_layout_ci.setLayoutCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pSetLayouts[i]);
  }
  for (u32 i{}; i < pipeline_layout_ci.pushConstantRangeCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pPushConstantRanges[i]);
  }

  auto it{pipeline_layouts_.find(hash_value)};
  if (it != pipeline_layouts_.end()) {
    return it->second;
  }

  vk::raii::PipelineLayout pipeline_layout{
      gpu_->CreatePipelineLayout(pipeline_layout_ci, name, index)};

  auto it1{pipeline_layouts_.emplace(hash_value, std::move(pipeline_layout))};

  return it1.first->second;
}

const vk::raii::ShaderModule& ComputeJob::RequestShaderModule(
    const vk::ShaderModuleCreateInfo& shader_module_ci, u64 hash_value,
    const std::string& name, i32 index) {
  auto it{shader_modules_.find(hash_value)};
  if (it != shader_modules_.end()) {
    return it->second;
  }

  vk::raii::ShaderModule shader_module{
      gpu_->CreateShaderModule(shader_module_ci, name, index)};

  auto it1{shader_modules_.emplace(hash_value, std::move(shader_module))};

  return it1.first->second;
}

const vk::raii::Pipeline& ComputeJob::RequestPipeline(
    const vk::ComputePipelineCreateInfo& compute_pipeline_ci, u64 hash_value,
    const std::string& name, i32 index) {
  auto it{pipelines_.find(hash_value)};
  if (it != pipelines_.end()) {
    return it->second;
  }

  vk::PipelineCacheCreateInfo pipeline_cache_ci;

  std::filesystem::path root_path{GetPath(LUKA_ROOT_PATH)};
  std::filesystem::path cache_path{root_path / ".cache" / "engine"};
  std::filesystem::path pipeline_cache_file{
      cache_path / ("pipeline_" + std::to_string(hash_value) + ".cache")};
  std::vector<u8> pipeline_cache_data;

  bool has_cache{};
  if (std::filesystem::exists(pipeline_cache_file)) {
    pipeline_cache_data = LoadBinaryU8(pipeline_cache_file);

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
      gpu_->CreatePipelineCache(pipeline_cache_ci, name, index)};

  vk::raii::Pipeline pipeline{
      gpu_->CreatePipeline(compute_pipeline_ci, pipeline_cache, name, index)};

  if (!has_cache) {
    std::vector<u8> pipeline_cache_data{pipeline_cache.getData()};
    if (!std::filesystem::exists(cache_path)) {
      std::filesystem::create_directories(cache_path);
    }
    SaveBinaryU8(pipeline_cache_data, pipeline_cache_file);
  }

  auto it1{pipelines_.emplace(hash_value, std::move(pipeline))};

  return it1.first->second;
}

}  // namespace luka::fw