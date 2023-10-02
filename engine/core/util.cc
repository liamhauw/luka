/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Util source file.
*/

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