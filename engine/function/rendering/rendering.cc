// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering()
    : asset_{gContext.asset},
      gpu_{gContext.gpu},
      function_ui_{gContext.function_ui} {
  // Pipeline.
  const std::vector<u8>& vertex_shader_buffer{asset_->GetVertexShaderBuffer()};
  const std::vector<u8>& fragment_shader_buffer{
      asset_->GetFragmentShaderBuffer()};
  uint32_t vertex_stride{sizeof(Vertex)};
  std::vector<std::pair<vk::Format, uint32_t>>
      vertex_input_attribute_format_offset{
          {vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)},
          {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)}};

  vk::PipelineLayoutCreateInfo pipeline_layout_ci;
  pipeline_layout_ = gpu_->CreatePipelineLayout(pipeline_layout_ci);

  vk::PipelineRenderingCreateInfo pipeline_rendering_ci{
      {}, color_formats_, depth_format_};

  pipeline_ =
      gpu_->CreatePipeline(vertex_shader_buffer, fragment_shader_buffer,
                           vertex_stride, vertex_input_attribute_format_offset,
                           pipeline_layout_, pipeline_rendering_ci);

  // Geometry.
  const std::vector<Vertex> vertices{{{-0.5F, -0.5F}, {1.0F, 1.0F, 0.0F}},
                                     {{0.5F, -0.5F}, {0.0F, 1.0F, 1.0F}},
                                     {{0.5F, 0.5F}, {1.0F, 0.0F, 1.0F}},
                                     {{-0.5F, 0.5F}, {1.0F, 1.0F, 1.0F}}};
  const std::vector<u16> indices = {0, 2, 1, 2, 0, 3};

  u64 vertices_size{vertices.size() * sizeof(Vertex)};
  u64 indices_size{indices.size() * sizeof(u16)};

  vk::BufferCreateInfo vertex_buffer_ci{
      {},
      vertices_size,
      vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
      vk::SharingMode::eExclusive};
  vk::BufferCreateInfo index_buffer_ci{
      {},
      indices_size,
      vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
      vk::SharingMode::eExclusive};

  vertex_buffer_ = gpu_->CreateBuffer(vertex_buffer_ci, false, vertices_size,
                                      vertices.data(), "vertex");
  vertex_buffer_ = gpu_->CreateBuffer(index_buffer_ci, false, indices_size,
                                      indices.data(), "index");

  // GBuffer.
  
}

void Rendering::Tick() {
  if (gContext.window->GetIconified()) {
    return;
  }

  const vk::raii::CommandBuffer& cur_command_buffer{gpu_->BeginFrame()};

  function_ui_->Render(cur_command_buffer);

  gpu_->EndFrame(cur_command_buffer);
}

}  // namespace luka
