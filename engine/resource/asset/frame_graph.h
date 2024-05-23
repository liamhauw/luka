// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"

namespace luka::ast {

enum class AttachmentType { kNone, kInput, kColor, kResolve, kDepthStencil };

struct Attachment {
  std::string name;
  vk::Format format;
  bool output;
};

struct Subpass {
  std::string name;
  std::unordered_map<AttachmentType, std::vector<u32>> attachments;
  std::string scene;
  std::vector<u32> lights;
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
  explicit FrameGraph(const std::filesystem::path& frame_graph_path);

  const std::vector<u32>& GetScenes() const;
  const std::vector<ast::Pass>& GetPasses() const;

 private:
  json json_;

  std::vector<u32> scenes_;
  std::vector<Pass> passes_;
};

}  // namespace luka::ast
