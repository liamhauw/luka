// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

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

  const std::vector<u8>& GetSource() const;

 private:
  std::vector<u8> source_;
};

}  // namespace ast

}  // namespace luka
