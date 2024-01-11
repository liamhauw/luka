// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

namespace ast {

class Shader {
 public:
  Shader() = default;
  Shader(const std::filesystem::path& shader_path);

  const std::vector<u8>& GetBuffer() const;

 private:
  std::vector<u8> buffer_;
};

}  // namespace ast

}  // namespace luka
