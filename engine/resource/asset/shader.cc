// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/shader.h"

#include "core/util.h"

namespace luka {

namespace ast {

Shader::Shader(const std::filesystem::path& shader_path) {
  source_ = LoadText(shader_path);
}

const std::string& Shader::GetSource() const { return source_; }

}  // namespace ast

}  // namespace luka
