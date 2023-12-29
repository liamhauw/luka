// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/mesh.h"

namespace luka {

namespace sg {

Mesh::Mesh(std::vector<Primitive>&& primitives, const std::string& name)
    : Component{name}, primitives_{std::move(primitives)} {}

std::type_index Mesh::GetType() { return typeid(Mesh); }

}  // namespace sg

}  // namespace luka
