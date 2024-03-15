// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"

namespace luka {

namespace ast {

enum class AttachmentType { kColor, kResolve, kDepthStencil };
struct AttachmentInfo {
  std::string name;
};

struct Subpass {
  std::string name;
  std::vector<u32> scenes;
  std::unordered_map<std::string, u32> shaders;
};

struct Pass {
  std::string name;
  std::unordered_map<AttachmentType, std::vector<AttachmentInfo>> attachments;
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
