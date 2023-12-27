// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/image.h"

namespace luka {

namespace sg {

Image::Image(gpu::Image&& image, vk::raii::ImageView&& image_view,
             const std::string& name)
    : Component{name},
      image_{std::move(image)},
      image_view_{std::move(image_view)} {}

std::type_index Image::GetType() { return typeid(Image); }

}  // namespace sg

}  // namespace luka
