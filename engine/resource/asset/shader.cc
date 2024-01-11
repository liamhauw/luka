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
  buffer_ = LoadBinary(shader_path);
}

const std::vector<u8>& Shader::GetBuffer() const { return buffer_; }

}  // namespace ast

}  // namespace luka
