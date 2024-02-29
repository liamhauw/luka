// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Context {
 public:
  Context(bool editor_mode = true);

  void Tick();

  void SetEditorMode(bool editor_mode);
  bool GetEditorMode() const;

 private:
  bool editor_mode_;
};

}  // namespace luka
