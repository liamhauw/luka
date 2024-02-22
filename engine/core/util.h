// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

template <typename T>
inline T AlignUp(T val, T alignment) {
  return (val + alignment - static_cast<T>(1)) &
         ~(alignment - static_cast<T>(1));
}

template <class T, class Y>
struct TypeCast {
  Y operator()(T value) const noexcept { return static_cast<Y>(value); }
};

template <typename T>
void HashCombine(u64& seed, const T& value) {
  std::hash<T> hasher;
  seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

std::string ReplacePathSlash(const std::string& str);

std::vector<u32> LoadBinary(const std::filesystem::path& binary_path);

std::string LoadText(const std::filesystem::path& text_path);

std::vector<f32> D2FVector(const std::vector<f64>& dvector);

}  // namespace luka
