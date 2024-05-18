// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/component.h"

namespace luka::ast::sc {

Component::Component(std::string name) : name_{std::move(name)} {}

const std::string& Component::GetName() const { return name_; }

}  // namespace luka::ast::sc
