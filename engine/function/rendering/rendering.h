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

class Rendering {
 public:
  Rendering();
  ~Rendering();

  void Tick();

  std::pair<const vk::raii::Sampler&, const vk::raii::ImageView&>
  GetViewportImage() const;

  void Render(const vk::raii::CommandBuffer& command_buffer);

 private:
  void Resize();

  void CreateModelResource();

  void CreatePipeline();
  void CreateGeometry();
  void CreateGBuffer();

  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Gpu> gpu_;

  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };

  // Model resource.
  std::vector<Buffer> model_buffer_staging_buffers_;
  std::vector<Buffer> model_image_staging_buffers_; 
  std::vector<Buffer> model_buffers_;
  std::vector<Image> model_images_;
  std::vector<vk::raii::ImageView> model_image_views_;
  std::vector<vk::raii::Sampler> model_samplers_;

  // Pipeline.
  vk::raii::PipelineLayout pipeline_layout_{nullptr};
  std::vector<vk::Format> color_formats_{vk::Format::eB8G8R8A8Unorm};
  vk::Format depth_format_{vk::Format::eUndefined};
  vk::raii::Pipeline pipeline_{nullptr};

  // Geometry.
  Buffer vertex_buffer_{nullptr};
  Buffer index_buffer_{nullptr};

  // GBuffer.
  vk::Extent2D extent_;
  std::vector<Image> color_images_;
  std::vector<vk::raii::ImageView> color_image_views_;
  Image depth_image_{nullptr};
  vk::raii::ImageView depth_image_view_{nullptr};
  vk::raii::Sampler sampler_{nullptr};
};

}  // namespace luka
