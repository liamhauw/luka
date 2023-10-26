/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "resource/asset/gltf.h"

#include <fstream>

#include "core/log.h"

namespace luka {

Gltf::Gltf(const std::filesystem::path& model_file_path) {
  // Parse model json.
  std::filesystem::path parent_path{model_file_path.parent_path()};

  std::ifstream model_file{model_file_path.string()};
  if (!model_file) {
    THROW("Fail to open {}", model_file_path.string());
  }
  mj_ = json::parse(model_file);

  // Load scenes.
  json::object_t mj{mj_};
  scene_ = GetElementUint32(mj, "scene", 0);
  if (mj_.find("scenes") != mj_.end()) {
    const json& scenes{mj_["scenes"]};
    scenes_.resize(scenes.size());
    for (uint32_t i{0}; i < scenes.size(); ++i) {
      const json& scene{scenes[i]};
      if (scene.find("nodes") != scene.end()) {
        const json& nodes{scene["nodes"]};
        for (uint32_t j{0}; j < nodes.size(); ++j) {
          scenes_[i].nodes.push_back(nodes[j].get<uint32_t>());
        }
      }
    }
  }

  // Load nodes.
  if (mj_.find("nodes") != mj_.end()) {
    const json& nodes{mj_["nodes"]};
    nodes_.resize(nodes.size());
    for (uint32_t i{0}; i < nodes.size(); ++i) {
      json::object_t node{nodes[i]};

      nodes_[i].name = GetElementString(node, "name", "");

      nodes_[i].mesh_index = GetElementUint32(node, "mesh", UINT32_MAX);
      nodes_[i].skin_index = GetElementUint32(node, "skin", UINT32_MAX);

      nodes_[i].transform.scale =
          GetElementVec4(node, "scale", {1.0, 1.0, 1.0, 0.0});
      nodes_[i].transform.rotation =
          GetElementVec4(node, "rotation", {0.0, 0.0, 0.0, 1.0});
      nodes_[i].transform.translation =
          GetElementVec4(node, "translation", {0.0, 0.0, 0.0, 0.0});
      nodes_[i].transform.matrix =
          GetElementMat4(node, "matrix", glm::mat4{1.0f});

      if (node.find("children") != node.end()) {
        for (uint32_t j{0}; j < node["children"].size(); ++j) {
          uint32_t id{node["children"][j]};
          nodes_[i].children.push_back(id);
        }
      }

      // TODO
      // Cameras and lights
    }
  }

  // Load meshes.
  const json& accessors{mj_["accessors"]};
  if (mj_.find("meshes") != mj_.end()) {
    const json& meshes{mj_["meshes"]};
    meshes_.resize(meshes.size());
    for (uint32_t i{0}; i < meshes.size(); ++i) {
      const json& primitives{meshes[i]["primitives"]};
      meshes_[i].primitives.resize(primitives.size());

      for (uint32_t j{0}; j < primitives.size(); ++j) {
        json::object_t primitive{primitives[j]};
        uint32_t position{
            GetElementUint32(primitive, "attributes/position", 0)};

        json::object_t accessor{accessors[position]};

        glm::vec4 max{GetElementVec4(accessor, "max", {0.0, 0.0, 0.0, 0.0})};
        glm::vec4 min{GetElementVec4(accessor, "min", {0.0, 0.0, 0.0, 0.0})};

        auto& pri{meshes_[i].primitives[j]};
        pri.center = (min + max) * 0.5f;
        pri.radius = max - pri.center;
        pri.center.y = 1.0f;
      }
    }
  }

  // Load buffers.
  if (mj_.find("buffers") != mj_.end()) {
    const json& buffers{mj_["buffers"]};
    buffers_.resize(buffers.size());
    for (uint32_t i{0}; i < buffers.size(); ++i) {
      json::object_t buffer{buffers[i]};

      std::string uri{GetElementString(buffer, "uri", "")};
      std::string uri_file_path{(parent_path / uri).string()};
      std::ifstream bin_file{uri_file_path, std::ios::in | std::ios::binary};

      // Get length.
      // bin_file.seekg(0, bin_file.end);
      // std::streamoff length{bin_file.tellg()};
      uint32_t length{GetElementUint32(buffer, "byteLength", 0)};

      bin_file.seekg(0, bin_file.beg);
      buffers_[i] = std::make_unique<char[]>(length);
      bin_file.read(buffers_[i].get(), length);
    }
  }

  // Load animations
  if (mj_.find("animations") != mj_.end()) {
    
  }

  // TODO 
  // Load cameras, skins and lights.
}

}  // namespace luka
