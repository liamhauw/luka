// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on
#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering() : gpu_{gContext.gpu} {
  vk::ImageCreateInfo image_ci{
      {},
      vk::ImageType::e2D,
      vk::Format::eR32G32B32A32Sfloat,
      {1, 1, 1},
      1,
      1,
      vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      vk::SharingMode::eExclusive,
      {},
      vk::ImageLayout::eUndefined};
  image_ = gpu_->CreateImage(image_ci);
}

void Rendering::Tick() {
  if (gContext.window->GetIconified()) {
    return;
  }
}

}  // namespace luka
