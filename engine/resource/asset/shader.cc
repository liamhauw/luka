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

Shader::Shader(const cfg::Shader& config_shader)
    : path_{config_shader.path.string()}, source_text_{LoadText(path_)} {}

// Shader::Shader(const std::filesystem::path& input_file_name,
//                EShLanguage language)
//     : path_{input_file_name.string()},
//       language_{language},
//       source_text_{LoadText(path_)} {}

u64 Shader::GetHashValue(const std::vector<std::string>& processes) const {
  std::vector<std::string> svec{processes};
  svec.push_back(path_);
  svec.push_back(source_text_);

  u64 hash_value{0};
  for (const std::string& str : svec) {
    HashCombine(hash_value, str);
  }
  return hash_value;
}

std::vector<u32> Shader::CompileToSpirv(
    const std::vector<std::string>& processes) const {
  std::string info_log;

  glslang::InitializeProcess();

  const char* source_string{source_text_.c_str()};
  EShMessages messages{static_cast<EShMessages>(
      EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules)};

  glslang::TShader shader{language_};
  shader.setStrings(&source_string, 1);

  std::string preamble;
  for (const std::string& process : processes) {
    std::string line{"#define " + process.substr(1) + "\n"};
    preamble += line;
  }
  shader.setPreamble(preamble.c_str());

  shader.setEntryPoint("main");
  shader.setSourceEntryPoint("main");
  shader.addProcesses(processes);
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
  if (!info_log.empty()) {
    LOGW("{}", info_log);
  }

  glslang::FinalizeProcess();

  return spirv;
}

}  // namespace ast

}  // namespace luka
