// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/scene_graph/component.h"

namespace luka {

namespace sg {

Component::Component(const std::string& name) : name_{name} {}

const std::string& Component::GetName() const { return name_; }

}  // namespace sg

}  // namespace luka
