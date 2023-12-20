// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

Component::Component(const std::string& name) { name_ = name; }

const std::string& Component::GetName() const { return name_; }

}  // namespace sg

}  // namespace luka
