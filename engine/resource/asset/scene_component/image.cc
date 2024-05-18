// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/image.h"

#include "core/log.h"

namespace luka::ast::sc {

Image::Image(gpu::Image&& image, vk::raii::ImageView&& image_view,
             const std::string& name)
    : Component{name},
      image_{std::move(image)},
      image_view_{std::move(image_view)} {}

Image::Image(const std::shared_ptr<Gpu>& gpu,
             const tinygltf::Image& tinygltf_image,
             const vk::raii::CommandBuffer& command_buffer,
             std::vector<gpu::Buffer>& staging_buffers)
    : Component{tinygltf_image.uri} {
  // Staging buffer.
  const auto& data{tinygltf_image.image};
  u64 data_size{data.size()};

  vk::BufferCreateInfo image_staging_buffer_ci{
      {}, data_size, vk::BufferUsageFlagBits::eTransferSrc};

  gpu::Buffer image_staging_buffer{
      gpu->CreateBuffer(image_staging_buffer_ci, data.data())};
  staging_buffers.push_back(std::move(image_staging_buffer));

  // Image.
  vk::Extent3D extent{static_cast<u32>(tinygltf_image.width),
                      static_cast<u32>(tinygltf_image.height), 1};
  u32 dim_count{0};
  vk::ImageType image_type{};
  if (extent.width >= 1) {
    ++dim_count;
  }
  if (extent.height >= 1) {
    ++dim_count;
  }
  if (extent.depth > 1) {
    ++dim_count;
  }
  switch (dim_count) {
    case 1:
      image_type = vk::ImageType::e1D;
      break;
    case 2:
      image_type = vk::ImageType::e2D;
      break;
    case 3:
      image_type = vk::ImageType::e3D;
      break;
    default:
      image_type = vk::ImageType::e2D;
      break;
  }

  vk::Format format{};
  if (tinygltf_image.component == 4 && tinygltf_image.bits == 8) {
    format = vk::Format::eR8G8B8A8Unorm;
  } else {
    THROW("Unsupport image format.");
  }
  u32 level_count{1};
  u32 layer_count{1};

  vk::ImageCreateInfo image_ci{
      {},
      image_type,
      format,
      extent,
      level_count,
      layer_count,
      vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst};

  image_ = gpu->CreateImage(image_ci, vk::ImageLayout::eShaderReadOnlyOptimal,
                            staging_buffers.back(), command_buffer,
                            tinygltf_image, GetName());

  // Image view.
  vk::ImageViewType image_view_type{};
  switch (image_type) {
    case vk::ImageType::e1D:
      image_view_type = vk::ImageViewType::e1D;
      break;
    case vk::ImageType::e2D:
      image_view_type = vk::ImageViewType::e2D;
      break;
    case vk::ImageType::e3D:
      image_view_type = vk::ImageViewType::e3D;
      break;
    default:
      image_view_type = vk::ImageViewType::e2D;
      break;
  }

  vk::ImageViewCreateInfo image_view_ci{
      {},
      *image_,
      image_view_type,
      format,
      {},
      {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
       VK_REMAINING_ARRAY_LAYERS}};

  image_view_ = gpu->CreateImageView(image_view_ci, GetName());
}

std::type_index Image::GetType() { return typeid(Image); }

const gpu::Image& Image::GetImage() const { return image_; }

const vk::raii::ImageView& Image::GetImageView() const { return image_view_; }

}  // namespace luka::ast::sc
