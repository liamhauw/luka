/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "core/json.h"
#include "core/math.h"

namespace luka {

struct GltfScene {
  std::vector<uint32_t> nodes;
};

struct Transform {
  glm::vec4 rotation;
  glm::vec4 translation;
  glm::vec4 scale;

  glm::mat4 matrix;
};

struct Node {
  std::string name;

  uint32_t skin_index;
  uint32_t mesh_index;

  Transform transform;

  std::vector<uint32_t> children;
};

struct Primitive {
  glm::vec4 center;
  glm::vec4 radius;
};

struct Mesh {
  std::vector<Primitive> primitives;
};

class Gltf {
 public:
  Gltf(const std::filesystem::path& model_file_path);

 private:
  json mj_;

  std::vector<GltfScene> scenes_;
  uint32_t scene_;
  std::vector<Node> nodes_;
  std::vector<Mesh> meshes_;
  std::vector<std::unique_ptr<char[]>> buffers_;
};

}  // namespace luka
