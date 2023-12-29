// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/scene.h"

namespace luka {

namespace sg {

Scene::Scene(const std::string& name) : Component{name} {}

std::type_index Scene::GetType() { return typeid(Scene); }

}  // namespace sg

}  // namespace luka
