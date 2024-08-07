// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/framework/spirv.h"

namespace luka::fw {

SPIRV::SPIRV(const ast::Shader& shader,
             const std::vector<std::string>& processes,
             vk::ShaderStageFlagBits stage, u64 hash_value)
    : spirv_{shader.CompileToSpirv(processes)},
      stage_{stage},
      hash_value_{hash_value} {
  spirv_cross::CompilerGLSL compiler{spirv_};
  ParseShaderResource(compiler);
  ParseSpecialization(compiler);
}

SPIRV::SPIRV(const std::vector<u32>& spirv, vk::ShaderStageFlagBits stage,
             u64 hash_value)
    : spirv_{spirv}, stage_{stage}, hash_value_{hash_value} {
  spirv_cross::CompilerGLSL compiler{spirv_};
  ParseShaderResource(compiler);
  ParseSpecialization(compiler);
}

vk::ShaderStageFlagBits SPIRV::GetStage() const { return stage_; }

u64 SPIRV::GetHashValue() const { return hash_value_; }

const std::vector<u32>& SPIRV::GetSpirv() const { return spirv_; }

const std::vector<ShaderResource>& SPIRV::GetShaderResources() const {
  return shader_resources_;
}

const std::vector<SpecializationConstant>& SPIRV::GetSpecializationConstants()
    const {
  return specialization_constants_;
}

void SPIRV::ParseShaderResource(const spirv_cross::CompilerGLSL& compiler) {
  spirv_cross::ShaderResources resources{compiler.get_shader_resources()};
  // Samplers.
  const auto& samplers{resources.separate_samplers};
  for (const auto& sampler : samplers) {
    ShaderResource shader_resource{};
    shader_resource.name = sampler.name;
    shader_resource.type = ShaderResourceType::kSampler;
    shader_resource.stage = stage_;
    shader_resource.set = ParseSet(compiler, sampler);
    shader_resource.binding = ParseBinding(compiler, sampler);
    shader_resource.array_size = ParseArraySize(compiler, sampler);

    shader_resources_.push_back(std::move(shader_resource));
  }

  // Combined image samplers.
  const auto& combined_image_samplers{resources.sampled_images};
  for (const auto& combined_image_sampler : combined_image_samplers) {
    ShaderResource shader_resource{};
    shader_resource.name = combined_image_sampler.name;
    shader_resource.type = ShaderResourceType::kCombinedImageSampler;
    shader_resource.stage = stage_;
    shader_resource.set = ParseSet(compiler, combined_image_sampler);
    shader_resource.binding = ParseBinding(compiler, combined_image_sampler);
    shader_resource.array_size =
        ParseArraySize(compiler, combined_image_sampler);

    shader_resources_.push_back(std::move(shader_resource));
  }

  // Sampled images.
  const auto& sampled_images{resources.separate_images};
  for (const auto& sampled_image : sampled_images) {
    ShaderResource shader_resource{};
    shader_resource.name = sampled_image.name;
    shader_resource.type = ShaderResourceType::kSampledImage;
    shader_resource.stage = stage_;
    shader_resource.set = ParseSet(compiler, sampled_image);
    shader_resource.binding = ParseBinding(compiler, sampled_image);
    shader_resource.array_size = ParseArraySize(compiler, sampled_image);

    shader_resources_.push_back(std::move(shader_resource));
  }

  // Storage images.
  const auto& storage_images{resources.storage_images};
  for (const auto& storage_image : storage_images) {
    ShaderResource shader_resource{};
    shader_resource.name = storage_image.name;
    shader_resource.type = ShaderResourceType::kStorageImage;
    shader_resource.stage = stage_;
    shader_resource.set = ParseSet(compiler, storage_image);
    shader_resource.binding = ParseBinding(compiler, storage_image);
    shader_resource.array_size = ParseArraySize(compiler, storage_image);

    shader_resources_.push_back(std::move(shader_resource));
  }

  // Uniform buffers.
  const auto& uniform_buffers{resources.uniform_buffers};
  for (const auto& uniform_buffer : uniform_buffers) {
    ShaderResource shader_resource{};
    shader_resource.name = uniform_buffer.name;
    shader_resource.type = ShaderResourceType::kUniformBuffer;
    shader_resource.stage = stage_;
    shader_resource.set = ParseSet(compiler, uniform_buffer);
    shader_resource.binding = ParseBinding(compiler, uniform_buffer);
    shader_resource.array_size = ParseArraySize(compiler, uniform_buffer);
    shader_resource.size = ParseSize(compiler, uniform_buffer);

    shader_resources_.push_back(std::move(shader_resource));
  }

  // Input attachments.
  const auto& input_attachments{resources.subpass_inputs};
  for (const auto& input_attachment : input_attachments) {
    ShaderResource shader_resource{};
    shader_resource.name = input_attachment.name;
    shader_resource.type = ShaderResourceType::kInputAttachment;
    shader_resource.stage = stage_;
    shader_resource.input_attachment_index =
        ParseInputAttachmentIndex(compiler, input_attachment);
    shader_resource.set = ParseSet(compiler, input_attachment);
    shader_resource.binding = ParseBinding(compiler, input_attachment);

    shader_resources_.push_back(std::move(shader_resource));
  }

  // Push constant buffers.
  const auto& push_constant_buffers{resources.push_constant_buffers};
  for (const auto& push_constant_buffer : push_constant_buffers) {
    ShaderResource shader_resource{};
    shader_resource.name = push_constant_buffer.name;
    shader_resource.type = ShaderResourceType::kPushConstantBuffer;
    shader_resource.stage = stage_;
    shader_resource.offset = ParseOffset(compiler, push_constant_buffer);
    shader_resource.size =
        ParseSize(compiler, push_constant_buffer) - shader_resource.offset;

    shader_resources_.push_back(std::move(shader_resource));
  }

  // Stage inputs.
  const auto& stage_inputs{resources.stage_inputs};
  for (const auto& stage_input : stage_inputs) {
    ShaderResource shader_resource{};
    shader_resource.name = stage_input.name;
    shader_resource.type = ShaderResourceType::kStageInput;
    shader_resource.stage = stage_;
    shader_resource.location = ParseLocation(compiler, stage_input);

    shader_resources_.push_back(std::move(shader_resource));
  }
}

void SPIRV::ParseSpecialization(const spirv_cross::CompilerGLSL& compiler) {
  const auto& specialization_constants{compiler.get_specialization_constants()};
  for (const auto& specialization_constant : specialization_constants) {
    SpecializationConstant sc{};
    sc.name = compiler.get_name(specialization_constant.id);
    sc.constant_id = specialization_constant.constant_id;

    specialization_constants_.push_back(sc);
  }
}

u32 SPIRV::ParseInputAttachmentIndex(const spirv_cross::CompilerGLSL& compiler,
                                     const spirv_cross::Resource& resource) {
  return compiler.get_decoration(resource.id,
                                 spv::DecorationInputAttachmentIndex);
}

u32 SPIRV::ParseSet(const spirv_cross::CompilerGLSL& compiler,
                    const spirv_cross::Resource& resource) {
  return compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
}

u32 SPIRV::ParseBinding(const spirv_cross::CompilerGLSL& compiler,
                        const spirv_cross::Resource& resource) {
  return compiler.get_decoration(resource.id, spv::DecorationBinding);
}

u32 SPIRV::ParseArraySize(const spirv_cross::CompilerGLSL& compiler,
                          const spirv_cross::Resource& resource) {
  const auto& spirv_type{compiler.get_type_from_variable(resource.id)};
  return spirv_type.array.size() ? spirv_type.array[0] : 1;
}

u32 SPIRV::ParseSize(const spirv_cross::CompilerGLSL& compiler,
                     const spirv_cross::Resource& resource) {
  const auto& spirv_type{compiler.get_type_from_variable(resource.id)};
  return compiler.get_declared_struct_size_runtime_array(spirv_type, 0);
}

u32 SPIRV::ParseOffset(const spirv_cross::CompilerGLSL& compiler,
                       const spirv_cross::Resource& resource) {
  const auto& spirv_type{compiler.get_type_from_variable(resource.id)};
  u32 offset{std::numeric_limits<std::uint32_t>::max()};
  for (u32 i{}; i < spirv_type.member_types.size(); ++i) {
    u32 mem_offset{compiler.get_member_decoration(spirv_type.self, i,
                                                  spv::DecorationOffset)};
    offset = std::min(offset, mem_offset);
  }

  return offset;
}

u32 SPIRV::ParseLocation(const spirv_cross::CompilerGLSL& compiler,
                         const spirv_cross::Resource& resource) {
  return compiler.get_decoration(resource.id, spv::DecorationLocation);
}

}  // namespace luka::fw
