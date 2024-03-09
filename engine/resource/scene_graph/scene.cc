// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/scene_graph/scene.h"

namespace luka {

namespace sg {

Scene::Scene(std::vector<Node*>&& nodes, const std::string& name)
    : Component{name}, nodes_{std::move(nodes)} {}

Scene::Scene(const std::vector<Node*> node_components,
             const tinygltf::Scene& model_scene)
    : Component{model_scene.name} {
  const std::vector<i32>& model_nodes{model_scene.nodes};
  for (u32 i{0}; i < model_nodes.size(); ++i) {
    Node* node{node_components[model_nodes[i]]};
    nodes_.push_back(node);
  }
}

std::type_index Scene::GetType() { return typeid(Scene); }

const std::vector<Node*>& Scene::GetNodes() const { return nodes_; }

}  // namespace sg

}  // namespace luka
