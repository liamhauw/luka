// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/log.h"
#include "engine.h"

luka::i32 main() {
  try {
    luka::Engine engine;
    engine.Run();
  } catch (const luka::Exception& e) {
    return -1;
  } catch (const std::exception& e) {
    LOGE("{}", e.what());
    return -1;
  } catch (...) {
    LOGE("Unknown exception.");
    return -1;
  }
  return 0;
}