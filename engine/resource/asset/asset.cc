/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Asset source file.
*/

#include "resource/asset/asset.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <filesystem>
#include <fstream>

#include "context.h"
#include "core/log.h"

namespace luka {

Asset::Asset() {
  LoadModel(gContext.config->GetModelFilePath().string(), model_);

  LoadShader(gContext.config->GetVertexShaderFilePath().string(),
             vertext_shader_buffer_);
  LoadShader(gContext.config->GetFragmentShaderFilePath().string(),
             fragment_shader_buffer_);
}

void Asset::Tick() {}

const tinygltf::Model& Asset::GetModel() const { return model_; }

const std::vector<char>& Asset::GetVertexShaderBuffer() const {
  return vertext_shader_buffer_;
}

const std::vector<char>& Asset::GetFragmentShaderBuffer() const {
  return fragment_shader_buffer_;
}

void Asset::LoadModel(const std::string& model_file_name,
                      tinygltf::Model& model) {
  tinygltf::TinyGLTF tiny_gltf;
  std::string err;
  std::string warn;
  bool result{
      tiny_gltf.LoadASCIIFromFile(&model_, &err, &warn, model_file_name)};
  if (!warn.empty()) {
    LOGW("tinygltf load warn: [{}].", warn);
  }
  if (!err.empty()) {
    LOGE("tinygltf load error: [{}].", err);
  }
  if (!result) {
    THROW("Fail to load gltf file.");
  }
}

void Asset::LoadShader(const std::string& shader_file_name,
                       std::vector<char>& shader_buffer) {
  std::ifstream shader_file(shader_file_name, std::ios::ate | std::ios::binary);
  if (!shader_file) {
    THROW("Fail to open {}", shader_file_name);
  }

  uint32_t file_size{static_cast<uint32_t>(shader_file.tellg())};
  shader_buffer.resize(file_size);
  shader_file.seekg(0);
  shader_file.read(shader_buffer.data(), file_size);

  shader_file.close();
}

}  // namespace luka
