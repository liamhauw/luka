/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Log header file.
*/

#pragma once

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <string>

namespace luka {

class Exception : public std::exception {
 public:
  Exception() {}
  Exception(const std::string& message) : message_{message} {}

  const char* what() const noexcept override { return message_.c_str(); }

 private:
  std::string message_;
};

#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) spdlog::error(__VA_ARGS__);
#define THROW(...)                    \
  LOGE("{}:{} ", __FILE__, __LINE__); \
  LOGE(__VA_ARGS__);                  \
  throw Exception {}

}  // namespace luka
