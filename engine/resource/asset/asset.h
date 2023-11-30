// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/gltf.h"

namespace luka {

class Config;

class Asset {
 public:
  Asset();

  void Tick();

  const std::vector<u8>& GetVertexShaderBuffer() const;
  const std::vector<u8>& GetFragmentShaderBuffer() const;

 private:
  void LoadShader(const std::string& shader_path,
                  std::vector<u8>& shader_buffer);

  // std::unique_ptr<Gltf> gltf_;
  std::vector<u8> vertext_shader_buffer_;
  std::vector<u8> fragment_shader_buffer_;
};

}  // namespace luka