// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/scene.h"

namespace luka {

namespace ast::sc {

Scene::Scene(std::vector<Node*>&& nodes, const std::string& name)
    : Component{name}, nodes_{std::move(nodes)} {}

Scene::Scene(const std::vector<Node*> node_components,
             const tinygltf::Scene& tinygltf_scene)
    : Component{tinygltf_scene.name} {
  const std::vector<i32>& tinygltf_nodes{tinygltf_scene.nodes};
  for (u32 i{0}; i < tinygltf_nodes.size(); ++i) {
    Node* node{node_components[tinygltf_nodes[i]]};
    nodes_.push_back(node);
  }
}

std::type_index Scene::GetType() { return typeid(Scene); }

const std::vector<Node*>& Scene::GetNodes() const { return nodes_; }

}  // namespace ast::sc

}  // namespace luka
