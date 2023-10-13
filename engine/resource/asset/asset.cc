/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Asset source file.
*/

#include "resource/asset/asset.h"

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

#include <filesystem>
#include <fstream>
#include <unordered_map>

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

void Asset::LoadObjModel(const std::string& model,
                         std::vector<Vertex>& vertices,
                         std::vector<uint32_t>& indices) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model.c_str());

  std::unordered_map<Vertex, uint32_t> unique_vertices{};

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Vertex vertex{};

      vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]};

      vertex.tex_coord = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (unique_vertices.count(vertex) == 0) {
        unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(unique_vertices[vertex]);
    }
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
