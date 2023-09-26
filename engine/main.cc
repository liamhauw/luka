/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Engine entry.
*/

#include <memory>

#include "core/log.h"
#include "engine.h"

int main() {
  try {
    luka::Engine::Run();
  } catch (const std::exception& e) {
    LOGE("std::exception: {}", e.what());
    return -1;
  }
  return 0;
}