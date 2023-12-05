// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Asset::Asset() : config_{gContext.config} {
  model_ = LoadModel(config_->GetModelFilePath());
  vertext_shader_buffer_ = LoadShader(config_->GetVertexShaderFilePath());
  fragment_shader_buffer_ = LoadShader(config_->GetFragmentShaderFilePath());
}

void Asset::Tick() {}

const tinygltf::Model& Asset::GetModel() const {
  return model_;
}

const std::vector<u8>& Asset::GetVertexShaderBuffer() const {
  return vertext_shader_buffer_;
}
const std::vector<u8>& Asset::GetFragmentShaderBuffer() const {
  return fragment_shader_buffer_;
}

tinygltf::Model Asset::LoadModel(const std::filesystem::path& model_path) {
  tinygltf::Model model;
  tinygltf::TinyGLTF tinygltf;
  std::string error;
  std::string warning;

  bool result{tinygltf.LoadASCIIFromFile(&model, &error, &warning,
                                         model_path.string())};
  if (!warning.empty()) {
    LOGW("Tinygltf: {}.", warning);
  }
  if (!error.empty()) {
    LOGE("Tinygltf: {}.", error);
  }
  if (!result) {
    THROW("Fail to load {}.", model_path.string());
  }

  return model;
}

std::vector<u8> Asset::LoadShader(const std::filesystem::path& shader_path) {
  std::vector<u8> shader_buffer;
  std::ifstream shader_file(shader_path.string(),
                            std::ios::ate | std::ios::binary);
  if (!shader_file) {
    THROW("Fail to open {}", shader_path.string());
  }
  uint32_t file_size{static_cast<uint32_t>(shader_file.tellg())};
  shader_buffer.resize(file_size);
  shader_file.seekg(0);
  shader_file.read(reinterpret_cast<char*>(shader_buffer.data()), file_size);
  shader_file.close();
  return shader_buffer;
}

}  // namespace luka
