/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Asset source file.
*/

#include "resource/asset/asset.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "context.h"
#include "core/log.h"

namespace luka {

Asset::Asset() {
  LoadObjModel(gContext.config->GetModelFilePath().string(), obj_model_);

  LoadShader(gContext.config->GetVertexShaderFilePath().string(),
             vertext_shader_buffer_);
  LoadShader(gContext.config->GetFragmentShaderFilePath().string(),
             fragment_shader_buffer_);
  LoadTexture(gContext.config->GetTextureFileFilePath().string(), texture_);
}

void Asset::Tick() {}

const ObjModel& Asset::GetObjModel() const { return obj_model_; }

const std::vector<char>& Asset::GetVertexShaderBuffer() const {
  return vertext_shader_buffer_;
}

const std::vector<char>& Asset::GetFragmentShaderBuffer() const {
  return fragment_shader_buffer_;
}

const Texture& Asset::GetTexture() const { return texture_; }

void Asset::FreeTexture() { stbi_image_free(texture_.piexls); }

void Asset::LoadObjModel(const std::string& model_path, ObjModel& obj_model) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  bool result{tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                               model_path.c_str())};
  if (!warn.empty()) {
    LOGW("tinyobj load warn: [{}].", warn);
  }
  if (!err.empty()) {
    LOGE("tinyobj load error: [{}].", err);
  }
  if (!result) {
    THROW("Fail to load obj file.");
  }

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
        unique_vertices[vertex] =
            static_cast<uint32_t>(obj_model.vertices.size());
        obj_model.vertices.push_back(vertex);
      }

      obj_model.indices.push_back(unique_vertices[vertex]);
    }
  }
}

void Asset::LoadShader(const std::string& shader_path,
                       std::vector<char>& shader_buffer) {
  std::ifstream shader_file(shader_path, std::ios::ate | std::ios::binary);
  if (!shader_file) {
    THROW("Fail to open {}", shader_path);
  }

  uint32_t file_size{static_cast<uint32_t>(shader_file.tellg())};
  shader_buffer.resize(file_size);
  shader_file.seekg(0);
  shader_file.read(shader_buffer.data(), file_size);

  shader_file.close();
}

void Asset::LoadTexture(const std::string& texture_path, Texture& texture) {
  texture.piexls =
      stbi_load(texture_path.c_str(), &texture.width, &texture.height,
                &texture.channels, STBI_rgb_alpha);
  if (!texture.piexls) {
    THROW("Fail to load texture image.");
  }
}

}  // namespace luka
