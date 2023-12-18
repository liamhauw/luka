// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component/light.h"

namespace luka {
namespace sg {

Light::Light(const LightProperty& property, const std::string& name)
    : property_{property}, Component{name} {}

std::type_index Light::GetType() { return typeid(Light); }

}  // namespace sg

}  // namespace luka
