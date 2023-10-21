/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw.
*/

#include "core/util.h"

#include <algorithm>

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