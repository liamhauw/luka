// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/rendering.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Rendering::Rendering() : asset_{gContext.asset}, gpu_{gContext.gpu} {
  CreateModelResource();

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

void Rendering::CreateModelResource() {
  const tinygltf::Model& model{asset_->GetModel()};

  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginTempCommandBuffer()};

  // Buffers.
  {
    u64 model_buffer_count{model.bufferViews.size()};

    vk::BufferCreateInfo model_buffer_ci{
        {},
        {},
        vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eIndexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive};

    vk::BufferCreateInfo model_buffer_staging_buffer_ci{
        {}, {}, vk::BufferUsageFlagBits::eTransferSrc};

    for (u64 i{0}; i < model_buffer_count; ++i) {
      const tinygltf::BufferView& buffer_view{model.bufferViews[i]};

      const std::vector<u8>& buffer{(model.buffers)[buffer_view.buffer].data};

      const u8* buffer_data{buffer.data() + buffer_view.byteOffset};
      u64 buffer_size{buffer_view.byteLength};

      model_buffer_ci.size = buffer_size;

      model_buffer_staging_buffer_ci.size = buffer_size;
      Buffer model_buffer_staging_buffer{
          gpu_->CreateBuffer(model_buffer_staging_buffer_ci, buffer_data)};
      model_buffer_staging_buffers_.push_back(
          std::move(model_buffer_staging_buffer));

      Buffer model_buffer{gpu_->CreateBuffer(
          model_buffer_ci, model_buffer_staging_buffers_[i], command_buffer)};

      model_buffers_.push_back(std::move(model_buffer));
    }
  }

  // Images and image views.
  {
    u64 model_image_count{model.images.size()};

    vk::BufferCreateInfo model_image_staging_buffer_ci{
        {}, {}, vk::BufferUsageFlagBits::eTransferSrc};
    vk::ImageCreateInfo model_image_ci{{}, vk::ImageType::e2D, {}, {}, 1, 1};
    vk::ImageViewCreateInfo model_image_view_ci{
        {}, {}, vk::ImageViewType::e2D,
        {}, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

    for (u64 i{0}; i < model_image_count; ++i) {
      const tinygltf::Image& image{model.images[i]};

      const std::string& name{image.name};
      vk::Format format;

      if (image.component == 4 && image.bits == 8) {
        format = vk::Format::eR8G8B8A8Unorm;
      } else {
        THROW("Unsupport image format.");
      }

      model_image_ci.format = format;
      model_image_ci.extent = vk::Extent3D{static_cast<u32>(image.width),
                                           static_cast<u32>(image.height), 1};
      model_image_ci.usage = vk::ImageUsageFlagBits::eSampled |
                             vk::ImageUsageFlagBits::eTransferDst;

      model_image_staging_buffer_ci.size = image.image.size();
      Buffer model_image_staging_buffer{gpu_->CreateBuffer(
          model_image_staging_buffer_ci, image.image.data())};
      model_image_staging_buffers_.push_back(
          std::move(model_image_staging_buffer));

      Image model_image{gpu_->CreateImage(
          model_image_ci, vk::ImageLayout::eShaderReadOnlyOptimal,
          model_image_staging_buffers_[i], command_buffer, name)};

      model_images_.push_back(std::move(model_image));

      model_image_view_ci.image = *model_images_[i];
      model_image_view_ci.format = format;

      vk::raii::ImageView image_view =
          gpu_->CreateImageView(model_image_view_ci, name);

      model_image_views_.push_back(std::move(image_view));
    }
  }

  // Samplers.
  {
    u64 model_sampler_count{model.samplers.size()};

    vk::SamplerCreateInfo sampler_ci;
    for (u64 i{0}; i < model_sampler_count; ++i) {
      const tinygltf::Sampler& sampler{model.samplers[i]};

      const std::string& name{sampler.name};

      switch (sampler.magFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
          sampler_ci.magFilter = vk::Filter::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
          sampler_ci.magFilter = vk::Filter::eLinear;
          break;
        default:
          sampler_ci.magFilter = vk::Filter::eNearest;
          break;
      }

      switch (sampler.minFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
          sampler_ci.minFilter = vk::Filter::eLinear;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
          sampler_ci.minFilter = vk::Filter::eLinear;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eLinear;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
          sampler_ci.minFilter = vk::Filter::eLinear;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eLinear;
          break;
        default:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
      }

      switch (sampler.wrapS) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eRepeat;
          break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eClampToEdge;
          break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eMirroredRepeat;
          break;
        default:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eRepeat;
          break;
      }

      switch (sampler.wrapT) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eRepeat;
          break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eClampToEdge;
          break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eMirroredRepeat;
          break;
        default:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eRepeat;
          break;
      }

      vk::raii::Sampler model_sampler{gpu_->CreateSampler(sampler_ci, name)};

      model_samplers_.push_back(std::move(model_sampler));
    }
  }

  gpu_->EndTempCommandBuffer(command_buffer);

  model_buffer_staging_buffers_.clear();
  model_image_staging_buffers_.clear();
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
  // const std::vector<Vertex> vertices{{{-0.5F, -0.5F}, {1.0F, 1.0F, 0.0F}},
  //                                    {{0.5F, -0.5F}, {0.0F, 1.0F, 1.0F}},
  //                                    {{0.5F, 0.5F}, {1.0F, 0.0F, 1.0F}},
  //                                    {{-0.5F, 0.5F}, {1.0F, 1.0F, 1.0F}}};
  // const std::vector<u16> indices = {0, 2, 1, 2, 0, 3};

  // u64 vertices_size{vertices.size() * sizeof(Vertex)};
  // u64 indices_size{indices.size() * sizeof(u16)};

  // vk::BufferCreateInfo vertex_buffer_ci{
  //     {},
  //     vertices_size,
  //     vk::BufferUsageFlagBits::eVertexBuffer |
  //         vk::BufferUsageFlagBits::eTransferDst,
  //     vk::SharingMode::eExclusive};
  // vk::BufferCreateInfo index_buffer_ci{
  //     {},
  //     indices_size,
  //     vk::BufferUsageFlagBits::eIndexBuffer |
  //         vk::BufferUsageFlagBits::eTransferDst,
  //     vk::SharingMode::eExclusive};

  // vertex_buffer_ = gpu_->CreateBuffer(vertex_buffer_ci, false, vertices_size,
  //                                     vertices.data(), "vertex");
  // index_buffer_ = gpu_->CreateBuffer(index_buffer_ci, false, indices_size,
  //                                    indices.data(), "index");
}

void Rendering::CreateGBuffer() {
  // extent_ = gpu_->GetExtent2D();
  // u64 color_image_count{color_formats_.size()};

  // {
  //   vk::ImageCreateInfo image_ci{{},
  //                                vk::ImageType::e2D,
  //                                {},
  //                                {extent_.width, extent_.height, 1},
  //                                1,
  //                                1,
  //                                vk::SampleCountFlagBits::e1,
  //                                vk::ImageTiling::eOptimal,
  //                                vk::ImageUsageFlagBits::eColorAttachment |
  //                                    vk::ImageUsageFlagBits::eSampled |
  //                                    vk::ImageUsageFlagBits::eStorage,
  //                                vk::SharingMode::eExclusive,
  //                                {},
  //                                vk::ImageLayout::eUndefined};

  //   vk::ImageViewCreateInfo image_view_ci{
  //       {},
  //       {},
  //       vk::ImageViewType::e2D,
  //       {},
  //       {},
  //       {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
  //        VK_REMAINING_ARRAY_LAYERS}};

  //   for (u64 i = 0; i < color_image_count; ++i) {
  //     image_ci.format = color_formats_[i];
  //     color_images_.push_back(std::move(
  //         gpu_->CreateImage(image_ci, vk::ImageLayout::eGeneral, 0, nullptr,
  //                           "g_color_" + std::to_string(i))));

  //     image_view_ci.image = *color_images_[i];
  //     image_view_ci.format = color_formats_[i];
  //     color_image_views_.push_back(std::move(gpu_->CreateImageView(
  //         image_view_ci, "g_color_" + std::to_string(i))));
  //   }
  // }

  // if (depth_format_ != vk::Format::eUndefined) {
  //   vk::ImageCreateInfo image_ci{
  //       {},
  //       vk::ImageType::e2D,
  //       {},
  //       {extent_.width, extent_.height, 1},
  //       1,
  //       1,
  //       vk::SampleCountFlagBits::e1,
  //       vk::ImageTiling::eOptimal,
  //       vk::ImageUsageFlagBits::eDepthStencilAttachment |
  //           vk::ImageUsageFlagBits::eSampled,
  //       vk::SharingMode::eExclusive,
  //       {},
  //       vk::ImageLayout::eUndefined};

  //   depth_image_ = gpu_->CreateImage(image_ci, vk::ImageLayout::eGeneral, 0,
  //                                    nullptr, "g_depth");

  //   vk::ImageViewCreateInfo image_view_ci{
  //       {},
  //       {},
  //       vk::ImageViewType::e2D,
  //       {},
  //       {},
  //       {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0,
  //        VK_REMAINING_ARRAY_LAYERS}};
  //   depth_image_view_ = gpu_->CreateImageView(image_view_ci, "g_depth");
  // }

  // vk::SamplerCreateInfo sampler_ci{{},
  //                                  vk::Filter::eLinear,
  //                                  vk::Filter::eLinear,
  //                                  vk::SamplerMipmapMode::eLinear,
  //                                  vk::SamplerAddressMode::eClampToBorder,
  //                                  vk::SamplerAddressMode::eClampToBorder,
  //                                  vk::SamplerAddressMode::eClampToBorder};
  // sampler_ = gpu_->CreateSampler(sampler_ci, "g");
}

}  // namespace luka
