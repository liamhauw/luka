/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Asset header file.
*/

#pragma once

#include <tiny_gltf.h>

#include <vector>

namespace luka {

class Asset {
 public:
  Asset();

  void Tick();

  const tinygltf::Model& GetModel() const;
  const std::vector<char>& GetVertexShaderBuffer() const;
  const std::vector<char>& GetFragmentShaderBuffer() const;

 private:
  void LoadModel(const std::string& model_file_name, tinygltf::Model& model);
  void LoadShader(const std::string& shader_file_name,
                  std::vector<char>& shader_buffer);

  tinygltf::Model model_;
  std::vector<char> vertext_shader_buffer_;
  std::vector<char> fragment_shader_buffer_;
};

}  // namespace luka