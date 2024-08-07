// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <spirv_glsl.hpp>

#include "resource/asset/asset.h"

namespace luka::fw {

enum class ShaderResourceType {
  kNone,
  kSampler,
  kCombinedImageSampler,
  kSampledImage,
  kStorageImage,
  kUniformBuffer,
  kStorageBuffer,
  kInputAttachment,
  kPushConstantBuffer,
  kStageInput
};

struct ShaderResource {
  std::string name;
  ShaderResourceType type;
  vk::ShaderStageFlags stage;
  u32 input_attachment_index;
  u32 set;
  u32 binding;
  u32 array_size;
  u64 size;
  u32 offset;
  u32 location;
};

struct SpecializationConstant {
  std::string name;
  u32 constant_id;
};

class SPIRV {
 public:
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

  static u32 ParseInputAttachmentIndex(
      const spirv_cross::CompilerGLSL& compiler,
      const spirv_cross::Resource& resource);
  static u32 ParseSet(const spirv_cross::CompilerGLSL& compiler,
                      const spirv_cross::Resource& resource);
  static u32 ParseBinding(const spirv_cross::CompilerGLSL& compiler,
                          const spirv_cross::Resource& resource);
  static u32 ParseArraySize(const spirv_cross::CompilerGLSL& compiler,
                            const spirv_cross::Resource& resource);
  static u32 ParseSize(const spirv_cross::CompilerGLSL& compiler,
                       const spirv_cross::Resource& resource);
  static u32 ParseOffset(const spirv_cross::CompilerGLSL& compiler,
                         const spirv_cross::Resource& resource);
  static u32 ParseLocation(const spirv_cross::CompilerGLSL& compiler,
                           const spirv_cross::Resource& resource);

  std::vector<u32> spirv_;
  vk::ShaderStageFlagBits stage_{};
  u64 hash_value_{};

  std::vector<ShaderResource> shader_resources_;
  std::vector<SpecializationConstant> specialization_constants_;
};

}  // namespace luka::fw
