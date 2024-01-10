// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/node.h"

namespace luka {

namespace sg {

Node::Node(glm::mat4&& matrix, Mesh* mesh, Light* light, Camera* camera,
           const std::vector<i32>& child_indices, const std::string& name)
    : Component{name},
      matrix_{std::move(matrix)},
      mesh_{mesh},
      light_{light},
      camera_{camera},
      child_indices_{child_indices} {}

std::type_index Node::GetType() { return typeid(Node); }

const std::vector<i32>& Node::GetChildIndices() const { return child_indices_; }

void Node::SetChildren(std::vector<Node*>&& children) { children_ = std::move(children); }

void Node::SetParent(Node* parent) { parent_ = parent; }

void Node::ClearParent() {
  parent_ = nullptr;
}

const std::vector<Node*>& Node::GetChildren() const {
  return children_;
}

Node* Node::GetParent() const {
  return parent_;
}

}  // namespace sg

}  // namespace luka
