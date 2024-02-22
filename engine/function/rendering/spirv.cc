// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/spirv.h"

namespace luka {

namespace rd {

SPIRV::SPIRV(const ast::Shader& shader,
             const std::vector<std::string>& processes,
             vk::ShaderStageFlagBits stage)
    : shader_{shader},
      processes_{processes},
      stage_{stage},
      spirv_{shader_.CompileToSpirv(processes_)} {
  ParseShaderResource();
}

const std::vector<u32>& SPIRV::GetSpirv() const { return spirv_; }

const std::vector<ShaderResource>& SPIRV::GetShaderResources() const {
  return shader_resources_;
}

void SPIRV::ParseShaderResource() {
  spirv_cross::CompilerGLSL compiler{spirv_};
  spirv_cross::ShaderResources resources{compiler.get_shader_resources()};

  // Uniform buffers.
  const auto& uniform_buffers{resources.uniform_buffers};
  for (const auto& uniform_buffer : uniform_buffers) {
    ShaderResource shader_resource;
    shader_resource.name = uniform_buffer.name;
    shader_resource.type = ShaderResourceType::kUniformBuffer;
    shader_resource.stage = stage_;
    shader_resource.set = ParseSet(compiler, uniform_buffer);
    shader_resource.binding = ParseBinding(compiler, uniform_buffer);
    shader_resource.array_size = ParseArraySize(compiler, uniform_buffer);
    shader_resource.size = ParseSize(compiler, uniform_buffer);

    shader_resources_.push_back(shader_resource);
  }

  // Sampled images.
  const auto& sampled_images{resources.sampled_images};
  for (const auto& sampled_image : sampled_images) {
    ShaderResource shader_resource;
    shader_resource.name = sampled_image.name;
    shader_resource.type = ShaderResourceType::kSampledImage;
    shader_resource.stage = stage_;
    shader_resource.set = ParseSet(compiler, sampled_image);
    shader_resource.binding = ParseBinding(compiler, sampled_image);
    shader_resource.array_size = ParseArraySize(compiler, sampled_image);

    shader_resources_.push_back(shader_resource);
  }

  // Push constant buffers.
  const auto& push_constant_buffers{resources.push_constant_buffers};
  for (const auto& push_constant_buffer : push_constant_buffers) {
    ShaderResource shader_resource;
    shader_resource.name = push_constant_buffer.name;
    shader_resource.type = ShaderResourceType::kPushConstantBuffer;
    shader_resource.stage = stage_;
    shader_resource.offset = ParseOffset(compiler, push_constant_buffer);
    shader_resource.size =
        ParseSize(compiler, push_constant_buffer) - shader_resource.offset;

    shader_resources_.push_back(shader_resource);
  }

  // Specializtion constants.
  const auto& specialization_constants{compiler.get_specialization_constants()};
  for (const auto& specialization_constant : specialization_constants) {
    ShaderResource shader_resource;
    shader_resource.name = compiler.get_name(specialization_constant.id);
    shader_resource.type = ShaderResourceType::kSpecializationConstant;
    shader_resource.stage = stage_;
    shader_resource.offset = 0;
    shader_resource.constant_id = specialization_constant.constant_id;

    const auto& spirv_type{compiler.get_type(
        compiler.get_constant(specialization_constant.id).constant_type)};
    switch (spirv_type.basetype) {
      case spirv_cross::SPIRType::BaseType::Boolean:
      case spirv_cross::SPIRType::BaseType::Char:
      case spirv_cross::SPIRType::BaseType::Int:
      case spirv_cross::SPIRType::BaseType::UInt:
      case spirv_cross::SPIRType::BaseType::Float:
        shader_resource.size = 4;
        break;
      case spirv_cross::SPIRType::BaseType::Int64:
      case spirv_cross::SPIRType::BaseType::UInt64:
      case spirv_cross::SPIRType::BaseType::Double:
        shader_resource.size = 8;
        break;
      default:
        shader_resource.size = 0;
        break;
    }

    shader_resources_.push_back(shader_resource);
  }
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
  for (u32 i{0}; i < spirv_type.member_types.size(); ++i) {
    u32 mem_offset{compiler.get_member_decoration(spirv_type.self, i,
                                                  spv::DecorationOffset)};
    offset = std::min(offset, mem_offset);
  }

  return offset;
}

}  // namespace rd

}  // namespace luka
