// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/math.h"
#include "resource/asset/scene_component/camera.h"
#include "resource/asset/scene_component/component.h"
#include "resource/asset/scene_component/light.h"
#include "resource/asset/scene_component/mesh.h"

namespace luka {

namespace ast::sc {

class Node : public Component {
 public:
  Node(glm::mat4&& tinygltf_matrix, const Mesh* mesh, const Light* light,
       const Camera* camera, const std::vector<i32>& child_indices,
       const std::string& name = {});

  Node(const std::vector<Light*>& light_components,
       const std::vector<Camera*>& camera_components,
       const std::vector<Mesh*>& mesh_components,
       const tinygltf::Node& tinygltf_node);

  virtual ~Node() = default;
  std::type_index GetType() override;

  void SetChildren(std::vector<Node*>&& children);
  void SetParent(Node* parent);
  void ClearParent();

  const std::vector<i32>& GetChildIndices() const;
  const std::vector<Node*>& GetChildren() const;
  const Node* GetParent() const;

  const glm::mat4& GetModelMarix() const;
  const Mesh* GetMesh() const;

 private:
  glm::mat4 model_matrix_;
  const Mesh* mesh_;
  const Light* light_;
  const Camera* camera_;
  std::vector<i32> child_indices_;
  std::vector<Node*> children_;

  const Node* parent_{nullptr};
};

}  // namespace ast::sc

}  // namespace luka
