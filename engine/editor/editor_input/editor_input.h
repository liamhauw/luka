// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Window;

class EditorInput {
 public:
  EditorInput();

  void Tick();

  void OnKey(i32 key, i32 scancode, i32 action, i32 mod);

 private:
  std::shared_ptr<Window> window_;
};

}  // namespace luka
