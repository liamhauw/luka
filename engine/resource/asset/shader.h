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

  u64 GetHashValue(const std::vector<std::string>& processes) const;

  std::vector<u32> CompileToSpirv(
      const std::vector<std::string>& processes) const;

 private:
  std::string input_file_name_;
  EShLanguage language_;
  std::string source_text_;
};

}  // namespace ast

}  // namespace luka
