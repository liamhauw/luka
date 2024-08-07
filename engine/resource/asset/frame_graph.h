// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"
#include "core/math.h"

namespace luka::ast {

enum class PassType { kNone, kGraphics, kCompute };

enum class AttachmentType { kNone, kInput, kColor, kResolve, kDepthStencil };

struct EnabledScene {
  u32 index;
  glm::mat4 model;
};

struct Attachment {
  std::string name;
  vk::Format format;
  bool output;
};

struct Subpass {
  std::string name;
  std::unordered_map<AttachmentType, std::vector<u32>> attachments;
  std::unordered_map<vk::ShaderStageFlagBits, u32> shaders;
  std::string scene;
  std::vector<u32> lights;
};

struct ComputeJob {
  u32 shader;
};

struct Pass {
  std::string name;
  PassType type;
  std::vector<Attachment> attachments;
  std::vector<Subpass> subpasses;
  ComputeJob compute_job;
};

class FrameGraph {
 public:
  FrameGraph() = default;
  explicit FrameGraph(const std::filesystem::path& frame_graph_path);

  const std::vector<EnabledScene>& GetEnabledScenes() const;
  const std::vector<ast::Pass>& GetPasses() const;

 private:
  json json_;

  std::vector<EnabledScene> enabled_scenes_;
  std::vector<Pass> passes_;
};

}  // namespace luka::ast
