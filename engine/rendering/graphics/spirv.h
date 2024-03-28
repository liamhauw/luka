// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <spirv_glsl.hpp>

#include "resource/asset/asset.h"

namespace luka {

namespace gs {

enum class ShaderResourceType {
  kNone,
  kSampler,
  kCombinedImageSampler,
  kSampledImage,
  kUniformBuffer,
  kInputAttachment,
  kPushConstantBuffer,
  kStageInput
};

struct ShaderResource {
  std::string name{""};
  ShaderResourceType type{ShaderResourceType::kNone};
  vk::ShaderStageFlags stage{vk::ShaderStageFlagBits::eAll};
  u32 input_attachment_index{UINT32_MAX};
  u32 set{UINT32_MAX};
  u32 binding{UINT32_MAX};
  u32 array_size{UINT32_MAX};
  u64 size{UINT64_MAX};
  u32 offset{UINT32_MAX};
  u32 location{UINT32_MAX};
};

struct SpecializationConstant {
  std::string name{""};
  u32 constant_id{UINT32_MAX};
};

class SPIRV {
 public:
  SPIRV() = default;

  SPIRV(const ast::Shader& shader, const std::vector<std::string>& processes,
        vk::ShaderStageFlagBits stage, u64 hash_value);

  SPIRV(const std::vector<u32>& spirv, vk::ShaderStageFlagBits stage,
        u64 hash_value);

  vk::ShaderStageFlagBits GetStage() const;
  u64 GetHashValue() const;

  const std::vector<u32>& GetSpirv() const;
  const std::vector<ShaderResource>& GetShaderResources() const;
  const std::vector<SpecializationConstant>& GetSpecializationConstants() const;

 private:
  void ParseShaderResource(const spirv_cross::CompilerGLSL& compiler);
  void ParseSpecialization(const spirv_cross::CompilerGLSL& compiler);

  u32 ParseInputAttachmentIndex(const spirv_cross::CompilerGLSL& compiler,
                                const spirv_cross::Resource& resource);
  u32 ParseSet(const spirv_cross::CompilerGLSL& compiler,
               const spirv_cross::Resource& resource);
  u32 ParseBinding(const spirv_cross::CompilerGLSL& compiler,
                   const spirv_cross::Resource& resource);
  u32 ParseArraySize(const spirv_cross::CompilerGLSL& compiler,
                     const spirv_cross::Resource& resource);
  u32 ParseSize(const spirv_cross::CompilerGLSL& compiler,
                const spirv_cross::Resource& resource);
  u32 ParseOffset(const spirv_cross::CompilerGLSL& compiler,
                  const spirv_cross::Resource& resource);
  u32 ParseLocation(const spirv_cross::CompilerGLSL& compiler,
                    const spirv_cross::Resource& resource);

  ast::Shader shader_;
  std::vector<std::string> processes_;

  std::vector<u32> spirv_;

  vk::ShaderStageFlagBits stage_;
  u64 hash_value_;

  std::vector<ShaderResource> shader_resources_;
  std::vector<SpecializationConstant> specialization_constants_;
};

}  // namespace gs

}  // namespace luka
