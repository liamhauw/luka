/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Log header file.
*/

#pragma once

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace luka {

#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGE(...) spdlog::error(__VA_ARGS__);
#define THROW(...)                      \
  LOGE("[{}:{}]:", __FILE__, __LINE__); \
  throw std::runtime_error { "" }

}  // namespace luka
