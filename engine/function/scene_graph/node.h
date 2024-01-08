// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/math.h"
#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Mesh;
class Light;
class Camera;

class Node : public Component {
 public:
  Node(glm::mat4&& matrix, Mesh* mesh, Light* light, Camera* camera,
       const std::vector<i32>& child_indices, const std::string& name = {});
  virtual ~Node() = default;
  std::type_index GetType() override;
  
  const std::vector<i32>& GetChildIndices() const;
  void SetChildren(std::vector<Node*>&& children);

  void SetParent(Node* parent);
  void ClearParent();
  
  const std::vector<Node*>& GetChildren() const;
  Node* GetParent() const;

 private:
  glm::mat4 matrix_;
  Mesh* mesh_;
  Light* light_;
  Camera* camera_;
  std::vector<i32> child_indices_;
  std::vector<Node*> children_;

  Node* parent_{nullptr};
};

}  // namespace sg

}  // namespace luka
