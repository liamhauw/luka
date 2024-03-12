// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

namespace ast {

struct Subpass {
  std::string name;
  std::vector<u32> scenes;
  std::unordered_map<std::string, u32> shaders;
};

struct Pass {
  std::string name;
  std::vector<u32> subpasses;
};

class FrameGraph {
 public:
  FrameGraph() = default;

  FrameGraph(const std::filesystem::path& frame_graph_path);
};

}  // namespace ast

}  // namespace luka
