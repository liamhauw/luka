// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"
#include "function/scene_graph/node.h"

namespace luka {

namespace sg {

class Scene {
 public:
  Scene(const std::string& name = {});

  void SetComponents(const std::type_index& type_info,
                     std::vector<std::unique_ptr<Component>>&& components);

  template <class T>
  void SetComponents(std::vector<std::unique_ptr<T>>&& components) {
    std::vector<std::unique_ptr<Component>> result(components.size());
    std::transform(
        components.begin(), components.end(), result.begin(),
        [](std::unique_ptr<T>& component) -> std::unique_ptr<Component> {
          return std::unique_ptr<Component>(std::move(component));
        });
    SetComponents(typeid(T), std::move(result));
  }

 private:
  std::string name_;
  std::vector<std::unique_ptr<Node>> nodes_;
  Node* root_{nullptr};
  std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>>
      components_;
};
}  // namespace sg

}  // namespace luka
