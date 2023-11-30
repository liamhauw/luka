// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on
#include "resource/asset/asset.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Asset::Asset() {
  LoadShader(gContext.config->GetVertexShaderFilePath().string(),
             vertext_shader_buffer_);
  LoadShader(gContext.config->GetFragmentShaderFilePath().string(),
             fragment_shader_buffer_);
}

void Asset::Tick() {}

const std::vector<u8>& Asset::GetVertexShaderBuffer() const {
  return vertext_shader_buffer_;
}
const std::vector<u8>& Asset::GetFragmentShaderBuffer() const {
  return fragment_shader_buffer_;
}

void Asset::LoadShader(const std::string& shader_path,
                       std::vector<u8>& shader_buffer) {
  std::ifstream shader_file(shader_path, std::ios::ate | std::ios::binary);
  if (!shader_file) {
    THROW("Fail to open {}", shader_path);
  }

  uint32_t file_size{static_cast<uint32_t>(shader_file.tellg())};
  shader_buffer.resize(file_size);
  shader_file.seekg(0);
  shader_file.read(reinterpret_cast<char*>(shader_buffer.data()), file_size);

  shader_file.close();
}

}  // namespace luka
