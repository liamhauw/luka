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

  void CreatePipeline();
  void CreateModelResource();
  void CreateGBuffer();

  struct UniformData {
    glm::dmat4 m;
    glm::dmat4 vp;
    glm::dvec4 eye;
    glm::dvec4 light;
  };

  struct alignas(16) MaterialData {
    glm::dmat4 model_mat4;
    glm::dmat4 inv_model_mat4;
    glm::dvec4 base_color_factor;
    double matallic_factor;
    double roughness_factor;
    double nomal_scale;
    double occlusion_strength;
    glm::dvec3 emissive_factor;
  };

  struct DrawElement {
    vk::IndexType index_type;
    u32 index_count;
    u32 index_buffer_index;
    u32 index_buffer_offset;

    u32 postion_buffer_index;
    u32 position_buffer_offset;

    u32 texcoord0_buffer_index;
    u32 texcoord0_buffer_offset;

    u32 normal_buffer_index;
    u32 normal_buffer_offset;

    u32 tangent_buffer_index;
    u32 tangent_buffer_offset;

    vk::raii::DescriptorSet descriptor_set{nullptr};
    MaterialData material_data;
    Buffer material_buffer{nullptr};
  };

  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Gpu> gpu_;

  // Pipeline.
  vk::raii::DescriptorSetLayout descriptor_set_layout_{nullptr};
  vk::raii::PipelineLayout pipeline_layout_{nullptr};
  std::vector<vk::Format> color_formats_{vk::Format::eB8G8R8A8Unorm};
  vk::Format depth_format_{vk::Format::eD32Sfloat};
  vk::raii::Pipeline pipeline_{nullptr};
  UniformData uniform_data_;
  Buffer uniform_buffer_{nullptr};

  // Model resource.
  Buffer dummy_buffer_staging_buffer_{nullptr};
  Buffer dummy_buffer_{nullptr};
  Buffer dummy_image_staging_buffer_{nullptr};
  Image dummy_image_{nullptr};
  vk::raii::ImageView dummy_image_view_{nullptr};
  vk::raii::Sampler dummy_sampler_{nullptr};

  std::vector<Buffer> model_buffer_staging_buffers_;
  std::vector<Buffer> model_buffers_;
  std::vector<Buffer> model_image_staging_buffers_;
  std::vector<Image> model_images_;
  std::vector<vk::raii::ImageView> model_image_views_;
  std::vector<vk::raii::Sampler> model_samplers_;
  std::vector<DrawElement> draw_elements_;

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
