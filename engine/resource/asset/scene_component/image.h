// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "base/gpu/gpu.h"
#include "base/gpu/image.h"
#include "resource/asset/scene_component/component.h"

namespace luka::ast::sc {

class Image : public Component {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Image)

  Image(gpu::Image&& image, vk::raii::ImageView&& image_view,
        const std::string& name = {});
  Image(const std::shared_ptr<Gpu>&, const tinygltf::Image& tinygltf_image,
        const vk::raii::CommandBuffer& command_buffer,
        std::vector<gpu::Buffer>& staging_buffers);

  ~Image() override = default;

  std::type_index GetType() override;

  const gpu::Image& GetImage() const;
  const vk::raii::ImageView& GetImageView() const;

 private:
  gpu::Image image_;
  vk::raii::ImageView image_view_{nullptr};
};

}  // namespace luka::ast::sc
