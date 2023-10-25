/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Engine entry.
*/

#include "core/log.h"
#include "engine.h"

int main() {
  try {
    luka::Engine::Run();
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