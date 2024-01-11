// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Context {
 public:
  Context(bool editor_mode, bool load);

  void Tick();

  void SetEditorMode(bool editor_mode);
  void SetLoad(bool load);

  bool GetEditorMode() const;
  bool GetLoad() const;

 private:
  bool editor_mode_;
  bool load_;
};

}  // namespace luka
