// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"

namespace luka {

namespace ast {

enum class AttachmentType { kInput, kColor, kResolve, kDepthStencil };

struct Attachment {
  std::string name;
  vk::Format format;
  bool output;
};

struct Subpass {
  std::string name;
  std::unordered_map<AttachmentType, std::vector<u32>> attachments;
  std::vector<u32> scenes;
  std::unordered_map<vk::ShaderStageFlagBits, u32> shaders;
  std::vector<std::string> inputs;
};

struct Pass {
  std::string name;
  std::vector<Attachment> attachments;
  std::vector<Subpass> subpasses;
};

class FrameGraph {
 public:
  FrameGraph() = default;

  FrameGraph(const std::filesystem::path& frame_graph_path);

  const std::vector<ast::Pass>& GetPasses() const { return passes_; }

 private:
  json json_;
  std::vector<Pass> passes_;
};

}  // namespace ast

}  // namespace luka
