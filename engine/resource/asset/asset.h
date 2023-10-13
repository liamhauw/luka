/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Asset header file.
*/

#pragma once

#include <stb_image.h>
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>

#include <string>
#include <vector>

#include "core/math.h"

namespace luka {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 tex_coord;

  bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color &&
           tex_coord == other.tex_coord;
  }
};

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class Asset {
 public:
  Asset();

  void Tick();

  const tinygltf::Model& GetModel() const;
  const std::vector<char>& GetVertexShaderBuffer() const;
  const std::vector<char>& GetFragmentShaderBuffer() const;
  
  void LoadObjModel(const std::string& model, std::vector<Vertex>& vertices,
                    std::vector<uint32_t>& indices);

 private:
  void LoadModel(const std::string& model_file_name, tinygltf::Model& model);
  void LoadShader(const std::string& shader_file_name,
                  std::vector<char>& shader_buffer);

  tinygltf::Model model_;
  std::vector<char> vertext_shader_buffer_;
  std::vector<char> fragment_shader_buffer_;
};

}  // namespace luka

namespace std {
template <>
struct hash<luka::Vertex> {
  size_t operator()(const luka::Vertex& vertex) const {
    return ((hash<glm::vec3>()(vertex.pos) ^
             (hash<glm::vec3>()(vertex.color) << 1)) >>
            1) ^
           (hash<glm::vec2>()(vertex.tex_coord) << 1);
  }
};
}  // namespace std