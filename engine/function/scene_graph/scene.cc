// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene.h"

namespace luka {

namespace sg {

Scene::Scene(const std::string& name) { name_ = name; }

void Scene::SetComponents(
    const std::type_index& type_info,
    std::vector<std::unique_ptr<Component>>&& components) {
  components_[type_info] = std::move(components);
}

const std::vector<std::unique_ptr<Component>>& Scene::GetComponents(
    const std::type_index& type_info) const {
  return components_.at(type_info);
}

bool Scene::HasComponent(const std::type_index& type_info) const {
  auto iter{components_.find(type_info)};
  return (iter != components_.end() && !iter->second.empty());
}

}  // namespace sg

}  // namespace luka
