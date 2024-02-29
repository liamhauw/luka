// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/context/context.h"

namespace luka {

Context::Context(bool editor_mode) : editor_mode_{editor_mode} {}

void Context::Tick() {}

void Context::SetEditorMode(bool editor_mode) { editor_mode_ = editor_mode; }

bool Context::GetEditorMode() const { return editor_mode_; }

}  // namespace luka
