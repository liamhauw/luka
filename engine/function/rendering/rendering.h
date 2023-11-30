// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/math.h"
#include "function/gpu/gpu.h"

namespace luka {

class Asset;
class FunctionUI;

class Rendering {
 public:
  Rendering();

  void Tick();

 private:
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<FunctionUI> function_ui_;

  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };
  vk::raii::PipelineLayout pipeline_layout_{nullptr};
  std::vector<vk::Format> color_formats_{vk::Format::eR8G8B8A8Unorm};
  vk::Format depth_format_{vk::Format::eUndefined};
  vk::raii::Pipeline pipeline_{nullptr};

  Buffer vertex_buffer_{nullptr};
  Buffer index_buffer_{nullptr};
};

}  // namespace luka
