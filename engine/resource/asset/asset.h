// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

// If delete this and declare Config, there will be a intellisense error.
// But if i remove tiny_gltf.h, everything will be ok.
#include "resource/config/config.h"
#include "tiny_gltf.h"
// I think there is a header containment conflict, which editor can't fix.

namespace luka {

class Asset {
 public:
  Asset();

  void Tick();

  const tinygltf::Model& GetModel() const;
  const std::vector<u8>& GetVertexShaderBuffer() const;
  const std::vector<u8>& GetFragmentShaderBuffer() const;

 private:
  tinygltf::Model LoadModel(const std::filesystem::path& model_path);
  std::vector<u8> LoadShader(const std::filesystem::path& shader_path);

  std::shared_ptr<Config> config_;

  tinygltf::Model model_;
  std::vector<u8> vertext_shader_buffer_;
  std::vector<u8> fragment_shader_buffer_;
};

}  // namespace luka