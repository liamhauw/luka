// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

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

std::string ReplacePathSlash(const std::string& str);

std::vector<u8> LoadBinary(const std::filesystem::path& binary_path);

std::string LoadText(const std::filesystem::path& text_path);

std::vector<f32> D2FVector(const std::vector<f64>& dvector);

}  // namespace luka
