// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <shaderc/shaderc.hpp>

namespace luka {

namespace ast {

class Shader {
 public:
  Shader() = default;

  Shader(const std::filesystem::path& input_file_name,
         shaderc_shader_kind shader_kind);

  std::vector<u32> Compile(
      std::vector<std::pair<std::string, std::string>> macro_definaitons = {},
      bool optimize = false) const;

 private:
  std::string input_file_name_;
  shaderc_shader_kind shader_kind_;
  std::string source_text_;
};

}  // namespace ast

}  // namespace luka
