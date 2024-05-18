// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/node.h"

#include "core/util.h"

namespace luka::ast::sc {

Node::Node(glm::mat4&& tinygltf_matrix, const Mesh* mesh, const Light* light,
           const Camera* camera, const std::vector<i32>& child_indices,
           const std::string& name)
    : Component{name},
      model_matrix_{tinygltf_matrix},
      mesh_{mesh},
      light_{light},
      camera_{camera},
      child_indices_{child_indices} {}

Node::Node(const std::vector<Light*>& light_components,
           const std::vector<Camera*>& camera_components,
           const std::vector<Mesh*>& mesh_components,
           const tinygltf::Node& tinygltf_node)
    : Component{tinygltf_node.name} {
  // Matrix.
  model_matrix_ = glm::mat4{1.0F};

  if (!tinygltf_node.matrix.empty()) {
    std::transform(tinygltf_node.matrix.begin(), tinygltf_node.matrix.end(),
                   glm::value_ptr(model_matrix_), TypeCast<f64, f32>{});
  } else {
    glm::vec3 scale{1.0F, 1.0F, 1.0F};
    glm::quat rotation{1.0F, 0.0F, 0.0F, 0.0F};
    glm::vec3 translation{0.0F, 0.0F, 0.0F};

    if (!tinygltf_node.scale.empty()) {
      std::transform(tinygltf_node.scale.begin(), tinygltf_node.scale.end(),
                     glm::value_ptr(scale), TypeCast<f64, f32>{});
    }
    if (!tinygltf_node.rotation.empty()) {
      std::transform(tinygltf_node.rotation.begin(),
                     tinygltf_node.rotation.end(), glm::value_ptr(rotation),
                     TypeCast<f64, f32>{});
    }
    if (!tinygltf_node.translation.empty()) {
      std::transform(tinygltf_node.translation.begin(),
                     tinygltf_node.translation.end(),
                     glm::value_ptr(translation), TypeCast<f64, f32>{});
    }

    model_matrix_ = glm::translate(glm::mat4(1.0F), translation) *
                    glm::mat4_cast(rotation) *
                    glm::scale(glm::mat4(1.0F), scale);
  }

  // Mesh.
  mesh_ = nullptr;
  if (tinygltf_node.mesh != -1) {
    mesh_ = mesh_components[tinygltf_node.mesh];
  }

  // Light.
  light_ = nullptr;
  auto light_iter{tinygltf_node.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION)};
  if (light_iter != tinygltf_node.extensions.end()) {
    i32 light_index{light_iter->second.Get("light").Get<i32>()};
    light_ = light_components[light_index];
  }

  // Camera.
  camera_ = nullptr;
  if (tinygltf_node.camera != -1) {
    camera_ = camera_components[tinygltf_node.camera];
  }

  // Children.
  child_indices_ = tinygltf_node.children;
}

std::type_index Node::GetType() { return typeid(Node); }

void Node::SetChildren(std::vector<Node*>&& children) {
  children_ = std::move(children);
}

void Node::SetParent(Node* parent) { parent_ = parent; }

void Node::ClearParent() { parent_ = nullptr; }

const std::vector<i32>& Node::GetChildIndices() const { return child_indices_; }

const std::vector<Node*>& Node::GetChildren() const { return children_; }

const Node* Node::GetParent() const { return parent_; }

const glm::mat4& Node::GetModelMarix() const { return model_matrix_; }

const Mesh* Node::GetMesh() const { return mesh_; }

}  // namespace luka::ast::sc
