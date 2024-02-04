// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/util.h"

#include "core/log.h"

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

std::vector<u8> LoadBinary(const std::filesystem::path& binary_path) {
  std::vector<u8> binary_data;

  std::ifstream binary_file(binary_path.string(),
                            std::ios::ate | std::ios::binary);
  if (!binary_file) {
    THROW("Fail to open {}", binary_path.string());
  }

  u32 size{static_cast<u32>(binary_file.tellg())};
  binary_data.resize(size);
  binary_file.seekg(0);
  binary_file.read(reinterpret_cast<char*>(binary_data.data()), size);
  binary_file.close();

  return binary_data;
}

std::string LoadText(const std::filesystem::path& text_path) {
  std::string text_data;

  std::ifstream text_file(text_path.string(), std::ios::in);

  if (!text_file) {
    THROW("Fail to open {}", text_path.string());
  }

  text_data = {std::istreambuf_iterator<char>{text_file},
               std::istreambuf_iterator<char>{}};

  return text_data;
}

std::vector<f32> D2FVector(const std::vector<f64>& dvector) {
  std::vector<f32> fvector(dvector.begin(), dvector.end());
  return fvector;
}

}  // namespace luka