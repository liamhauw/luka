// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

Subpass::Subpass(const vk::raii::RenderPass& render_pass, u32 frame_count)
    : render_pass_{*render_pass}, frame_count_{frame_count} {}

std::vector<DrawElement>& Subpass::GetDrawElements() { return draw_elements_; }

}  // namespace rd

}  // namespace luka
