// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/shader.h"

#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/Logger.h>
#include <glslang/Public/ResourceLimits.h>

#include "core/log.h"
#include "core/util.h"

namespace luka {

namespace ast {

Shader::Shader(const std::filesystem::path& input_file_name,
               EShLanguage language)
    : input_file_name_{input_file_name.string()},
      language_{language},
      source_text_{LoadText(input_file_name_)} {}

std::vector<u32> Shader::CompileToSpirv() const {
  std::string info_log;

  glslang::InitializeProcess();

  const char* source_string{source_text_.c_str()};
  EShMessages messages{static_cast<EShMessages>(
      EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules)};

  glslang::TShader shader{language_};
  shader.setStrings(&source_string, 1);
  shader.setPreamble("");
  shader.setEntryPoint("main");
  shader.setSourceEntryPoint("main");
  shader.addProcesses({});
  if (!shader.parse(GetDefaultResources(), 100, false, messages)) {
    info_log = std::string{shader.getInfoLog()} + "\n" +
               std::string{shader.getInfoDebugLog()};
    THROW("{}", info_log);
  }

  glslang::TProgram program;
  program.addShader(&shader);
  if (!program.link(messages)) {
    info_log = std::string{program.getInfoLog()} + "\n" +
               std::string{program.getInfoDebugLog()};
    THROW("{}", info_log);
  }

  glslang::TIntermediate* intermediate{program.getIntermediate(language_)};

  std::vector<std::uint32_t> spirv;
  spv::SpvBuildLogger logger;
  glslang::GlslangToSpv(*intermediate, spirv, &logger);

  info_log = logger.getAllMessages();
  LOGI("{}", info_log);

  glslang::FinalizeProcess();

  return spirv;
}

}  // namespace ast

}  // namespace luka
