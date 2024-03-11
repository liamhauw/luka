// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/asset/scene_component/component.h"
#include "resource/gpu/gpu.h"
#include "resource/gpu/image.h"

namespace luka {

namespace ast::sc {

class Image : public Component {
 public:
  Image(gpu::Image&& image, vk::raii::ImageView&& image_view,
        const std::string& name = {});

  Image(std::shared_ptr<Gpu> gpu, const tinygltf::Image& tinygltf_image,
        const vk::raii::CommandBuffer& command_buffer,
        std::vector<gpu::Buffer>& staging_buffers);

  virtual ~Image() = default;
  std::type_index GetType() override;

  const gpu::Image& GetImage() const;
  const vk::raii::ImageView& GetImageView() const;

 private:
  gpu::Image image_;
  vk::raii::ImageView image_view_{nullptr};
};

}  // namespace ast::sc

}  // namespace luka
