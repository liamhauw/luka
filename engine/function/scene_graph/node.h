// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/math.h"
#include "function/scene_graph/camera.h"
#include "function/scene_graph/component.h"
#include "function/scene_graph/light.h"
#include "function/scene_graph/mesh.h"

namespace luka {

namespace sg {

class Node : public Component {
 public:
  Node(glm::mat4&& model_matrix, Mesh* mesh, Light* light, Camera* camera,
       const std::vector<i32>& child_indices, const std::string& name = {});

  Node(const std::vector<Light*>& light_components,
       const std::vector<Camera*>& camera_components,
       const std::vector<Mesh*>& mesh_components,
       const tinygltf::Node& model_node);

  virtual ~Node() = default;
  std::type_index GetType() override;

  void SetChildren(std::vector<Node*>&& children);
  void SetParent(Node* parent);
  void ClearParent();

  const std::vector<i32>& GetChildIndices() const;
  const std::vector<Node*>& GetChildren() const;
  Node* GetParent() const;

  const glm::mat4& GetModelMarix() const;
  const Mesh* GetMesh() const;

 private:
  glm::mat4 model_matrix_;
  Mesh* mesh_;
  Light* light_;
  Camera* camera_;
  std::vector<i32> child_indices_;
  std::vector<Node*> children_;

  Node* parent_{nullptr};
};

}  // namespace sg

}  // namespace luka
