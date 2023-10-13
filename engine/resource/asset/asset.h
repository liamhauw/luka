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

struct ObjModel {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct Texture {
  int width;
  int height;
  int channels;
  stbi_uc* piexls;
};

class Asset {
 public:
  Asset();

  void Tick();

  const tinygltf::Model& GetGltfModel() const;
  const ObjModel& GetObjModel() const;
  const std::vector<char>& GetVertexShaderBuffer() const;
  const std::vector<char>& GetFragmentShaderBuffer() const;
  const Texture& GetTexture() const;
  void FreeTexture();

 private:
  void LoadGltfModel(const std::string& model_path,
                     tinygltf::Model& gltf_model);
  void LoadObjModel(const std::string& model_path, ObjModel& obj_model);
  void LoadShader(const std::string& shader_path,
                  std::vector<char>& shader_buffer);
  void LoadTexture(const std::string& texture_path, Texture& texture);

  tinygltf::Model gltf_model_;
  ObjModel obj_model_;
  std::vector<char> vertext_shader_buffer_;
  std::vector<char> fragment_shader_buffer_;
  Texture texture_;
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