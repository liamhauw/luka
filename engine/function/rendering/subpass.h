// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/context.h"
#include "resource/asset/asset.h"

namespace luka {

namespace rd {

class Subpass {
 public:
  Subpass(std::shared_ptr<Asset> asset, Context& context);

 private:
  const ast::Shader* vertex_;
  const ast::Shader* fragment_;
  Context* context_;
};

}  // namespace rd

}  // namespace luka
