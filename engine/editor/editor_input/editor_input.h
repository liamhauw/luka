// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/window/window.h"
#include "function/context/context.h"

namespace luka {

class EditorInput {
 public:
  EditorInput(std::shared_ptr<Context> context, std::shared_ptr<Window> window);

  void Tick();

  void OnKey(i32 key, i32 scancode, i32 action, i32 mod);

 private:
  std::shared_ptr<Context> context_;
  std::shared_ptr<Window> window_;
};

}  // namespace luka
