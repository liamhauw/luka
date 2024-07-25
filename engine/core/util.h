// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#define DELETE_SPECIAL_MEMBER_FUNCTIONS(class_name)  \
  class_name(const class_name&) = delete;            \
  class_name(class_name&&) = delete;                 \
  class_name& operator=(const class_name&) = delete; \
  class_name& operator=(class_name&&) = delete;

namespace luka {

template <typename T, typename Y>
struct TypeCast {
  Y operator()(T value) const noexcept { return static_cast<Y>(value); }
};

template <typename T>
void HashCombine(u64& seed, const T& value) {
  std::hash<T> hasher;
  seed ^= hasher(value) + 0x9e3779b9 + (seed << 6U) + (seed >> 2U);
}

std::filesystem::path GetPath(const std::string& path);

std::vector<u8> LoadBinaryU8(const std::filesystem::path& binary_path);
std::vector<u32> LoadBinaryU32(const std::filesystem::path& binary_path);

void SaveBinaryU8(const std::vector<u8>& binary_data,
                  const std::filesystem::path& binary_path);
void SaveBinaryU32(const std::vector<u32>& binary_data,
                   const std::filesystem::path& binary_path);

std::string LoadText(const std::filesystem::path& text_path);
void SaveText(const std::filesystem::path& text_path,
              const std::string& text_data);

std::vector<f32> D2FVector(const std::vector<f64>& dvector);

std::vector<std::vector<u32>> SplitVector(const std::vector<u32>& input);

std::string ToLower(const std::string& str);

}  // namespace luka
