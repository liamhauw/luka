// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/util.h"

namespace luka {

std::string ReplacePathSlash(const std::string& str) {
  std::string res{str};
  if (PATH_SEPARATOR == '/') {
    std::replace(res.begin(), res.end(), '\\', '/');
  } else {
    std::replace(res.begin(), res.end(), '/', '\\');
  }
  return res;
}

}  // namespace luka