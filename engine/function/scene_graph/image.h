// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/gpu/image.h"
#include "function/scene_graph/component.h"
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

 private:
  gpu::Image image_{nullptr};
  vk::raii::ImageView image_view_{nullptr};
};

}  // namespace sg

}  // namespace luka
