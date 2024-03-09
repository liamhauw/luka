// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/gpu/gpu.h"
#include "resource/gpu/image.h"
#include "resource/scene_graph/component.h"
#include "resource/asset/image.h"

namespace luka {

namespace sg {

class Image : public Component {
 public:
  Image(gpu::Image&& image, vk::raii::ImageView&& image_view,
        const std::string& name = {});

  Image(std::shared_ptr<Gpu> gpu, const ast::Image& model_image,
        const vk::raii::CommandBuffer& command_buffer,
        std::vector<gpu::Buffer>& staging_buffers);

  virtual ~Image() = default;
  std::type_index GetType() override;

  const gpu::Image& GetImage() const;
  const vk::raii::ImageView& GetImageView() const;

 private:
  gpu::Image image_{nullptr};
  vk::raii::ImageView image_view_{nullptr};
};

}  // namespace sg

}  // namespace luka
