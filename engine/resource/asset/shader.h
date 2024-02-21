// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "glslang/Public/ShaderLang.h"

namespace luka {

namespace ast {

class Shader {
 public:
  Shader() = default;

  Shader(const std::filesystem::path& input_file_name, EShLanguage language);

  std::vector<u32> CompileToSpirv() const;

 private:
  std::string input_file_name_;
  EShLanguage language_;
  std::string source_text_;
};

}  // namespace ast

}  // namespace luka
