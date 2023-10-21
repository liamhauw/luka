/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw.
*/

#pragma once

#include <memory>

namespace luka {

class Window;

class EditorInput {
 public:
  EditorInput();

  void Tick();

  void OnKey(int key, int scancode, int action, int mod);

 private:
  std::shared_ptr<Window> window_;
};

}  // namespace luka
