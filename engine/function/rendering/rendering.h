/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Rendering header file.
*/

#pragma once

#include <memory>

#include "function/rendering/instance.h"

namespace luka {

class Window;

class Rendering {
 public:
  Rendering();

  void Tick();

 private:
  std::shared_ptr<Window> window_;

  Instance instance_;
};

}  // namespace luka
