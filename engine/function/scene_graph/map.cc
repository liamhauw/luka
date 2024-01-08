// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/map.h"

#include "core/log.h"
#include "function/scene_graph/node.h"
#include "function/scene_graph/scene.h"

namespace luka {

namespace sg {

Map::Map(const std::string& name) { name_ = name; }

void Map::SetSupportedExtensions(
    std::unordered_map<std::string, bool>&& supported_extensions) {
  supported_extensions_ = supported_extensions;
}

const std::unordered_map<std::string, bool>& Map::GetSupportedExtensions()
    const {
  return supported_extensions_;
}

void Map::SetDefaultScene(i32 default_scene) { default_scene_ = default_scene; }

void Map::LoadScene(i32 scene) {
  i32 scene_index{-1};

  auto scene_components{GetComponents<sg::Scene>()};
  i32 scene_cunt{static_cast<i32>(scene_components.size())};
  if (scene >= 0 && scene < scene_cunt) {
    scene_index = scene;
  } else if (default_scene_ != -1) {
    scene_index = default_scene_;
  } else {
    THROW("Fail to load scene.");
  }

  auto node_components{GetComponents<sg::Node>()};
  for (sg::Node* node : node_components) {
    node->ClearParent();
  }

  sg::Scene* cur_scene{scene_components[scene_index]};
  const std::vector<sg::Node*>& nodes{cur_scene->GetNodes()};

  sg::Node* root_node{nullptr};
  std::queue<std::pair<sg::Node*, sg::Node*>> traverse_nodes;
  for (sg::Node* node : nodes) {
    traverse_nodes.push(std::make_pair(root_node, node));
  }

  while (!traverse_nodes.empty()) {
    auto iter{traverse_nodes.front()};
    traverse_nodes.pop();

    sg::Node* parent{iter.first};
    sg::Node* child{iter.second};

    child->SetParent(parent);

    const std::vector<sg::Node*> child_children{child->GetChildren()};
    for (sg::Node* child_child : child_children) {
      traverse_nodes.push(std::make_pair(child, child_child));
    }
  }
}

void Map::SetComponents(const std::type_index& type_info,
                        std::vector<std::unique_ptr<Component>>&& components) {
  components_[type_info] = std::move(components);
}

const std::vector<std::unique_ptr<Component>>& Map::GetComponents(
    const std::type_index& type_info) const {
  return components_.at(type_info);
}

bool Map::HasComponent(const std::type_index& type_info) const {
  auto iter{components_.find(type_info)};
  return (iter != components_.end() && !iter->second.empty());
}

}  // namespace sg

}  // namespace luka
