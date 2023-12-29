// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Map {
 public:
  Map(const std::string& name = {});

  template <typename T>
  void SetComponents(std::vector<std::unique_ptr<T>>&& components) {
    std::vector<std::unique_ptr<Component>> result(components.size());
    std::transform(
        components.begin(), components.end(), result.begin(),
        [](std::unique_ptr<T>& component) -> std::unique_ptr<Component> {
          return std::unique_ptr<Component>(std::move(component));
        });
    SetComponents(typeid(T), std::move(result));
  }

  template <typename T>
  std::vector<T*> GetComponents() const {
    std::vector<T*> result;
    if (HasComponent(typeid(T))) {
      auto& scene_components{GetComponents(typeid(T))};

      result.resize(scene_components.size());
      std::transform(scene_components.begin(), scene_components.end(),
                     result.begin(),
                     [](const std::unique_ptr<Component>& component) -> T* {
                       return dynamic_cast<T*>(component.get());
                     });
    }
    return result;
  }

 private:
  void SetComponents(const std::type_index& type_info,
                     std::vector<std::unique_ptr<Component>>&& components);

  const std::vector<std::unique_ptr<Component>>& GetComponents(
      const std::type_index& type_info) const;

  bool HasComponent(const std::type_index& type_info) const;

  std::string name_;
  std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>>
      components_;
};
}  // namespace sg

}  // namespace luka
