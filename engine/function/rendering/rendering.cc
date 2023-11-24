// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on
#include "function/rendering/rendering.h"

#include "context.h"

namespace luka {

Rendering::Rendering() {}

void Rendering::Tick() {
  if (gContext.window->GetIconified()) {
    return;
  }
}

}  // namespace luka
