// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/light.h"

namespace luka {
namespace sg {

Light::Light(const LightProperty& property, const std::string& name)
    : Component{name}, property_{property} {}

std::type_index Light::GetType() { return typeid(Light); }

}  // namespace sg

}  // namespace luka
