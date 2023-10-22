/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <string>

namespace luka {

template <typename T>
inline T AlignUp(T val, T alignment) {
  return (val + alignment - static_cast<T>(1)) &
         ~(alignment - static_cast<T>(1));
}

std::string ReplacePathSlash(const std::string& str);
}  // namespace luka
