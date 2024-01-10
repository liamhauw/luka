// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene.h"

namespace luka {

namespace sg {

Scene::Scene(std::vector<Node*>&& nodes, const std::string& name)
    : Component{name}, nodes_{std::move(nodes)} {}

std::type_index Scene::GetType() { return typeid(Scene); }

const std::vector<Node*>& Scene::GetNodes() const { return nodes_; }

}  // namespace sg

}  // namespace luka
