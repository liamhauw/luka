// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/node.h"

#include "core/util.h"

namespace luka {

namespace sg {

Node::Node(glm::mat4&& model_matrix, Mesh* mesh, Light* light, Camera* camera,
           const std::vector<i32>& child_indices, const std::string& name)
    : Component{name},
      model_matrix_{std::move(model_matrix)},
      mesh_{mesh},
      light_{light},
      camera_{camera},
      child_indices_{child_indices} {}

Node::Node(const std::vector<Light*>& light_components,
           const std::vector<Camera*>& camera_components,
           const std::vector<Mesh*>& mesh_components,
           const tinygltf::Node& model_node)
    : Component{model_node.name} {
  // Matrix.
  model_matrix_ = glm::mat4{1.0F};

  if (!model_node.matrix.empty()) {
    std::transform(model_node.matrix.begin(), model_node.matrix.end(),
                   glm::value_ptr(model_matrix_), TypeCast<f64, f32>{});
  } else {
    glm::vec3 scale{1.0F, 1.0F, 1.0F};
    glm::quat rotation{1.0F, 0.0F, 0.0F, 0.0F};
    glm::vec3 translation{0.0F, 0.0F, 0.0F};

    if (!model_node.scale.empty()) {
      std::transform(model_node.scale.begin(), model_node.scale.end(),
                     glm::value_ptr(scale), TypeCast<f64, f32>{});
    }
    if (!model_node.rotation.empty()) {
      std::transform(model_node.rotation.begin(), model_node.rotation.end(),
                     glm::value_ptr(rotation), TypeCast<f64, f32>{});
    }
    if (!model_node.translation.empty()) {
      std::transform(model_node.translation.begin(),
                     model_node.translation.end(), glm::value_ptr(translation),
                     TypeCast<f64, f32>{});
    }

    model_matrix_ = glm::translate(glm::mat4(1.0F), translation) *
                    glm::mat4_cast(rotation) *
                    glm::scale(glm::mat4(1.0F), scale);
  }

  // Mesh.
  mesh_ = nullptr;
  if (model_node.mesh != -1) {
    mesh_ = mesh_components[model_node.mesh];
  }

  // Light.
  light_ = nullptr;
  auto light_iter{model_node.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION)};
  if (light_iter != model_node.extensions.end()) {
    i32 light_index{light_iter->second.Get("light").Get<i32>()};
    light_ = light_components[light_index];
  }

  // Camera.
  camera_ = nullptr;
  if (model_node.camera != -1) {
    camera_ = camera_components[model_node.camera];
  }

  // Children.
  child_indices_ = model_node.children;
}

std::type_index Node::GetType() { return typeid(Node); }

void Node::SetChildren(std::vector<Node*>&& children) {
  children_ = std::move(children);
}

void Node::SetParent(Node* parent) { parent_ = parent; }

void Node::ClearParent() { parent_ = nullptr; }

const std::vector<i32>& Node::GetChildIndices() const { return child_indices_; }

const std::vector<Node*>& Node::GetChildren() const { return children_; }

Node* Node::GetParent() const { return parent_; }

const glm::mat4& Node::GetModelMarix() const { return model_matrix_; }

const Mesh* Node::GetMesh() const { return mesh_; }

}  // namespace sg

}  // namespace luka
