// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

Subpass::Subpass(std::shared_ptr<Asset> asset)
    : vertex_{&(asset->GetAssetInfo().vertex)},
      fragment_{&(asset->GetAssetInfo().fragment)} {}

const vk::raii::Pipeline& Subpass::GetPipeline() const { return pipeline_; }

const vk::Viewport& Subpass::GetViewport() const { return viewport_; }

const vk::Rect2D& Subpass::GetScissor() const { return scissor_; }

}  // namespace rd

}  // namespace luka
