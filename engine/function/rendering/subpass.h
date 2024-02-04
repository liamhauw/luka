// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

namespace luka {

namespace rd {

class Subpass {
 public:
  Subpass(std::shared_ptr<Asset> asset);
  virtual ~Subpass() = default;

  virtual void Draw(const vk::raii::CommandBuffer& command_buffer) = 0;

  const vk::raii::Pipeline& GetPipeline() const;
  const vk::Viewport& GetViewport() const;
  const vk::Rect2D& GetScissor() const;

 private:
  const ast::Shader* vertex_;
  const ast::Shader* fragment_;

  std::vector<u32> input_attachments_;
  std::vector<u32> output_attachments_{0};
  std::vector<u32> color_resolve_attachments_;

  vk::raii::Pipeline pipeline_{nullptr};
  vk::Viewport viewport_;
  vk::Rect2D scissor_;
};

}  // namespace rd

}  // namespace luka
