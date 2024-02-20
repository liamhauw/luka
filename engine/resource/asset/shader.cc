// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/shader.h"

#include "core/log.h"
#include "core/util.h"

namespace luka {

namespace ast {

Shader::Shader(const std::filesystem::path& input_file_name,
               shaderc_shader_kind shader_kind)
    : input_file_name_{input_file_name.string()},
      shader_kind_{shader_kind},
      source_text_{LoadText(input_file_name_)} {}

std::vector<u32> Shader::Compile(
    std::vector<std::pair<std::string, std::string>> macro_definaitons,
    bool optimize) const {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

  for (const auto& macro_definaiton : macro_definaitons) {
    const std::string& name{macro_definaiton.first};
    const std::string& value{macro_definaiton.second};
    if (value.empty()) {
      options.AddMacroDefinition(name);
    } else {
      options.AddMacroDefinition(name, value);
    }
  }

  if (optimize) {
    options.SetOptimizationLevel(shaderc_optimization_level_size);
  }

  shaderc::SpvCompilationResult result{compiler.CompileGlslToSpv(
      source_text_, shader_kind_, input_file_name_.c_str(), options)};

  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    THROW("Fail to compile {} to spirv", input_file_name_);
  }

  return std::vector<u32>{result.cbegin(), result.cend()};
}

}  // namespace ast

}  // namespace luka
