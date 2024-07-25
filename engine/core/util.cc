// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/util.h"

#include "core/log.h"

namespace luka {

std::filesystem::path GetPath(const std::string& path) {
  std::string res{path};
  if (PATH_SEPARATOR == '/') {
    std::replace(res.begin(), res.end(), '\\', '/');
  } else {
    std::replace(res.begin(), res.end(), '/', '\\');
  }
  return std::filesystem::path{res};
}

std::vector<u8> LoadBinaryU8(const std::filesystem::path& binary_path) {
  std::ifstream binary_file{binary_path.string(), std::ios::binary};
  if (!binary_file) {
    THROW("Fail to open {}", binary_path.string());
  }

  binary_file.seekg(0, std::ios::end);
  std::streampos file_size{binary_file.tellg()};
  binary_file.seekg(0, std::ios::beg);

  std::vector<u8> binary_data(file_size);
  binary_file.read(reinterpret_cast<char*>(binary_data.data()), file_size);

  if (binary_file.gcount() != file_size) {
    THROW("Fail to read {}", binary_path.string());
  }

  binary_file.close();

  return binary_data;
}

std::vector<u32> LoadBinaryU32(const std::filesystem::path& binary_path) {
  std::ifstream binary_file{binary_path.string(), std::ios::binary};
  if (!binary_file) {
    THROW("Fail to open {}", binary_path.string());
  }

  binary_file.seekg(0, std::ios::end);
  std::streampos file_size{binary_file.tellg()};
  binary_file.seekg(0, std::ios::beg);

  std::vector<u32> binary_data(file_size / 4);
  binary_file.read(reinterpret_cast<char*>(binary_data.data()), file_size);

  if (binary_file.gcount() != file_size) {
    THROW("Fail to read {}", binary_path.string());
  }

  binary_file.close();

  return binary_data;
}

void SaveBinaryU8(const std::vector<u8>& binary_data,
                  const std::filesystem::path& binary_path) {
  std::ofstream binary_file{binary_path.string(), std::ios::binary};
  if (!binary_file) {
    THROW("Fail to open {}", binary_path.string());
  }

  binary_file.write(reinterpret_cast<const char*>(binary_data.data()),
                    static_cast<i64>(binary_data.size()));
  binary_file.close();
}

void SaveBinaryU32(const std::vector<u32>& binary_data,
                   const std::filesystem::path& binary_path) {
  std::ofstream binary_file{binary_path.string(), std::ios::binary};
  if (!binary_file) {
    THROW("Fail to open {}", binary_path.string());
  }

  binary_file.write(reinterpret_cast<const char*>(binary_data.data()),
                    static_cast<i64>(binary_data.size()) * 4);
  binary_file.close();
}

std::string LoadText(const std::filesystem::path& text_path) {
  std::string text_data;

  std::ifstream text_file{text_path.string(), std::ios::in};

  if (!text_file) {
    THROW("Fail to open {}", text_path.string());
  }

  text_data = {std::istreambuf_iterator<char>{text_file},
               std::istreambuf_iterator<char>{}};

  return text_data;
}

void SaveText(const std::filesystem::path& text_path,
              const std::string& text_data) {
  std::ofstream text_file{text_path.string(), std::ios::out};

  if (!text_file) {
    THROW("Fail to open " + text_path.string());
  }

  text_file << text_data;
  text_file.close();
}

std::vector<f32> D2FVector(const std::vector<f64>& dvector) {
  std::vector<f32> fvector{dvector.begin(), dvector.end()};
  return fvector;
}

std::vector<std::vector<u32>> SplitVector(const std::vector<u32>& input) {
  std::vector<std::vector<u32>> result;
  if (input.empty()) {
    return result;
  }

  std::vector<u32> sorted{input};
  std::sort(sorted.begin(), sorted.end());

  std::vector<u32> current;
  current.push_back(sorted[0]);

  for (size_t i = 1; i < sorted.size(); ++i) {
    if (sorted[i] == (current.back() + 1)) {
      current.push_back(sorted[i]);
    } else {
      result.push_back(current);
      current.clear();
      current.push_back(sorted[i]);
    }
  }

  result.push_back(current);
  return result;
}

std::string ToLower(const std::string& str) {
  std::string result{str};
  std::transform(result.begin(), result.end(), result.begin(),
                 [](auto c) { return std::tolower(c); });
  return result;
}

}  // namespace luka