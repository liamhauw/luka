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
  vk::Pipeline pipeline;
  vk::PipelineLayout pipeline_layout;
  vk::raii::DescriptorSets descriptor_sets{nullptr};
  std::map<u32, const sg::VertexAttribute*> location_vertex_attributes;
  u64 vertex_count;
  const sg::IndexAttribute* index_attribute;
  bool has_index;
};

class Subpass {
 public:
  Subpass(std::shared_ptr<Asset> asset, std::shared_ptr<Gpu> gpu,
          std::shared_ptr<SceneGraph> scene_graph);
  virtual ~Subpass() = default;

  const std::vector<DrawElement>& GetDrawElements() const;

 protected:
  virtual void CreateDrawElements(const vk::raii::RenderPass& render_pass) = 0;

  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<SceneGraph> scene_graph_;

  std::vector<DrawElement> draw_elements_;

  std::map<u64, SPIRV> spirv_shaders_;
};

}  // namespace rd

}  // namespace luka
