// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/image.h"
#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Image : public Component {
 public:
  Image(gpu::Image&& image, vk::raii::ImageView&& image_view,
        const std::string& name = {});
  virtual ~Image() = default;
  std::type_index GetType() override;

 private:
  gpu::Image image_;
  vk::raii::ImageView image_view_;
};

}  // namespace sg

}  // namespace luka
