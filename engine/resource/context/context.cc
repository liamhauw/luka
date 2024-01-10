// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/context/context.h"

namespace luka {

Context::Context(bool editor_mode, bool load)
    : editor_mode_{editor_mode}, load_{load} {}

void Context::SetEditorMode(bool editor_mode) { editor_mode_ = editor_mode; }

void Context::SetLoad(bool load) { load_ = load; }

bool Context::GetEditorMode() const { return editor_mode_; }

bool Context::GetLoad() const { return load_; }

}  // namespace luka
