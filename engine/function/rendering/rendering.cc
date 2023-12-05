// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering() : asset_{gContext.asset}, gpu_{gContext.gpu} {
  const tinygltf::Model& model{asset_->GetModel()};

  CreatePipeline();
  CreateGeometry();
  CreateGBuffer();
}

Rendering::~Rendering() { gpu_->WaitIdle(); }

void Rendering::Tick() {
  if (gContext.window->GetIconified()) {
    return;
  }

  if (gContext.window->GetFramebufferResized()) {
    Resize();
    return;
  }
}

std::pair<const vk::raii::Sampler&, const vk::raii::ImageView&>
Rendering::GetViewportImage() const {
  return std::make_pair(std::ref(sampler_), std::ref(color_image_views_[0]));
}

void Rendering::Render(const vk::raii::CommandBuffer& command_buffer) {
  vk::RenderingAttachmentInfo attachment_info{
      {},
      vk::ImageLayout::eAttachmentOptimal,
      {},
      {},
      {},
      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore};

  std::vector<vk::RenderingAttachmentInfo> color_attachment_infos;
  attachment_info.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
  for (u64 i = 0; i < color_image_views_.size(); ++i) {
    attachment_info.imageView = *color_image_views_[i];
    color_attachment_infos.push_back(attachment_info);
  }

  vk::RenderingInfo rendering_info{
      {}, {{0, 0}, extent_}, 1, 0, color_attachment_infos, nullptr, nullptr};

  command_buffer.beginRendering(rendering_info);

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);

  command_buffer.setViewport(
      0, vk::Viewport{0.0f, 0.0f, static_cast<float>(extent_.width),
                      static_cast<float>(extent_.height), 0.0f, 1.0f});
  command_buffer.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, extent_});

  command_buffer.bindVertexBuffers(0, *vertex_buffer_, {0});
  command_buffer.bindIndexBuffer(*index_buffer_, 0, vk::IndexType::eUint16);

  command_buffer.drawIndexed(6, 1, 0, 0, 0);

  command_buffer.endRendering();
}

void Rendering::Resize() {
  gpu_->WaitIdle();
  color_image_views_.clear();
  color_images_.clear();
  depth_image_view_.clear();
  depth_image_.Clear();
  sampler_.clear();
  CreateGBuffer();
}

void Rendering::CreatePipeline() {
  const std::vector<u8>& vertex_shader_buffer{asset_->GetVertexShaderBuffer()};
  const std::vector<u8>& fragment_shader_buffer{
      asset_->GetFragmentShaderBuffer()};
  uint32_t vertex_stride{sizeof(Vertex)};
  std::vector<std::pair<vk::Format, uint32_t>>
      vertex_input_attribute_format_offset{
          {vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)},
          {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)}};

  vk::PipelineLayoutCreateInfo pipeline_layout_ci;
  pipeline_layout_ =
      gpu_->CreatePipelineLayout(pipeline_layout_ci, "rendering");

  vk::PipelineRenderingCreateInfo pipeline_rendering_ci{
      {}, color_formats_, depth_format_};

  pipeline_ = gpu_->CreatePipeline(
      vertex_shader_buffer, fragment_shader_buffer, vertex_stride,
      vertex_input_attribute_format_offset, pipeline_layout_,
      pipeline_rendering_ci, "rendering");
}

void Rendering::CreateGeometry() {
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
  index_buffer_ = gpu_->CreateBuffer(index_buffer_ci, false, indices_size,
                                     indices.data(), "index");
}

void Rendering::CreateGBuffer() {
  extent_ = gpu_->GetExtent2D();
  u64 color_image_count{color_formats_.size()};

  {
    vk::ImageCreateInfo image_ci{{},
                                 vk::ImageType::e2D,
                                 {},
                                 {extent_.width, extent_.height, 1},
                                 1,
                                 1,
                                 vk::SampleCountFlagBits::e1,
                                 vk::ImageTiling::eOptimal,
                                 vk::ImageUsageFlagBits::eColorAttachment |
                                     vk::ImageUsageFlagBits::eSampled |
                                     vk::ImageUsageFlagBits::eStorage,
                                 vk::SharingMode::eExclusive,
                                 {},
                                 vk::ImageLayout::eUndefined};

    vk::ImageViewCreateInfo image_view_ci{
        {},
        {},
        vk::ImageViewType::e2D,
        {},
        {},
        {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
         VK_REMAINING_ARRAY_LAYERS}};

    for (u64 i = 0; i < color_image_count; ++i) {
      image_ci.format = color_formats_[i];
      color_images_.push_back(std::move(
          gpu_->CreateImage(image_ci, vk::ImageLayout::eGeneral, 0, nullptr,
                            "g_color_" + std::to_string(i))));

      image_view_ci.image = *color_images_[i];
      image_view_ci.format = color_formats_[i];
      color_image_views_.push_back(std::move(gpu_->CreateImageView(
          image_view_ci, "g_color_" + std::to_string(i))));
    }
  }

  if (depth_format_ != vk::Format::eUndefined) {
    vk::ImageCreateInfo image_ci{
        {},
        vk::ImageType::e2D,
        {},
        {extent_.width, extent_.height, 1},
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment |
            vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        {},
        vk::ImageLayout::eUndefined};

    depth_image_ = gpu_->CreateImage(image_ci, vk::ImageLayout::eGeneral, 0,
                                     nullptr, "g_depth");

    vk::ImageViewCreateInfo image_view_ci{
        {},
        {},
        vk::ImageViewType::e2D,
        {},
        {},
        {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0,
         VK_REMAINING_ARRAY_LAYERS}};
    depth_image_view_ = gpu_->CreateImageView(image_view_ci, "g_depth");
  }

  vk::SamplerCreateInfo sampler_ci{{},
                                   vk::Filter::eLinear,
                                   vk::Filter::eLinear,
                                   vk::SamplerMipmapMode::eLinear,
                                   vk::SamplerAddressMode::eClampToBorder,
                                   vk::SamplerAddressMode::eClampToBorder,
                                   vk::SamplerAddressMode::eClampToBorder};
  sampler_ = gpu_->CreateSampler(sampler_ci, "g");
}

}  // namespace luka
