// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/subpass.h"

namespace luka {

namespace rd {

Subpass::Subpass(std::shared_ptr<Asset> asset, Context& context)
    : vertex_{&(asset->GetAssetInfo().vertex)},
      fragment_{&(asset->GetAssetInfo().fragment)},
      context_{&context} {}

}  // namespace rd

}  // namespace luka
