// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/rendering/spirv.h"
#include "function/scene_graph/scene_graph.h"
#include "resource/asset/asset.h"

namespace luka {

namespace rd {

struct DrawElement {
  glm::mat4 model;
  vk::Pipeline pipeline;
  vk::PipelineLayout pipeline_layout;
  std::vector<vk::DescriptorSet> descriptor_sets;

  std::map<vk::Buffer, sg::VertexAttribute> vertex_buffer_attributes;
  std::pair<vk::Buffer, sg::IndexAttribute> index_buffer_attribute;
  u64 vertex_count;
  bool has_index;
};

class Subpass {
 public:
  Subpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
          std::shared_ptr<SceneGraph> scene_graph);
  virtual ~Subpass() = default;

  const vk::raii::Pipeline& GetPipeline() const;
  const std::vector<DrawElement>& GetDrawElements() const;

 protected:
  virtual void CreatePipeline() = 0;
  virtual void CreateDrawElements() = 0;

  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<SceneGraph> scene_graph_;

  vk::raii::Pipeline pipeline_{nullptr};
  std::vector<DrawElement> draw_elements_;

  std::map<u64, SPIRV> spirv_shaders_;
};

}  // namespace rd

}  // namespace luka
