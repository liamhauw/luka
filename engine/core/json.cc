/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "core/json.h"

namespace luka {

glm::vec4 GetVec4(const json::array_t& json_array) {
  return glm::vec4{json_array[0], json_array[1], json_array[2],
                   (json_array.size() == 4) ? json_array[3] : 0};
}

glm::mat4 GetMat4(const json::array_t& json_array) {
  return glm::mat4(
      glm::vec4(json_array[0], json_array[1], json_array[2], json_array[3]),
      glm::vec4(json_array[4], json_array[5], json_array[6], json_array[7]),
      glm::vec4(json_array[8], json_array[9], json_array[10], json_array[11]),
      glm::vec4(json_array[12], json_array[13], json_array[14],
                json_array[15]));
}

template <class T>
T GetElement(const json::object_t* root, const std::string& path,
             const T& default_value) {
  std::istringstream path_stream{path};
  std::string token;
  while (std::getline(path_stream, token, '/')) {
    if (token.empty()) {
      continue;
    }
    auto it{root->find(token)};
    if (it == root->end()) {
      return default_value;
    }

    if (token.back() == ']') {
      size_t open_bracket_pos{token.find('[')};
      if (open_bracket_pos == std::string::npos) {
        return default_value;
      }

      size_t close_bracket_pos{token.find(']')};
      if (close_bracket_pos == std::string::npos ||
          close_bracket_pos <= open_bracket_pos + 1) {
        return default_value;
      }

      std::string index_str{token.substr(
          open_bracket_pos + 1, close_bracket_pos - open_bracket_pos - 1)};
      size_t i{std::stoull(index_str)};

      root = it->second.at(i).get_ptr<const json::object_t*>();
    } else {
      if (it->second.is_object())
        root = it->second.get_ptr<const json::object_t*>();
      else
        return it->second.get<T>();
    }
  }

  return default_value;
}

std::string GetElementString(const json::object_t& root,
                             const std::string& path,
                             const std::string& default_value) {
  return GetElement<std::string>(&root, path, default_value);
}

uint32_t GetElementUint32(const json::object_t& root, const std::string& path,
                          uint32_t default_value) {
  return GetElement<uint32_t>(&root, path, default_value);
}

json::array_t GetElementJsonArray(const json::object_t& root,
                                  const std::string& path,
                                  const json::array_t& default_value) {
  return GetElement<json::array_t>(&root, path, default_value);
}

glm::vec4 GetElementVec4(const json::object_t& root, const std::string& path,
                         const glm::vec4& default_value) {
  json::array_t dv{default_value.x, default_value.y, default_value.z,
                   default_value.w};
  return GetVec4(GetElement<json::array_t>(&root, path, dv));
}

glm::mat4 GetElementMat4(const json::object_t& root, const std::string& path,
                         const glm::mat4& default_value) {
  // clang-format off
  json::array_t dv{
      default_value[0][0], default_value[1][0], default_value[2][0], default_value[3][0], 
      default_value[0][1], default_value[1][1], default_value[2][1], default_value[3][1], 
      default_value[0][2], default_value[1][2], default_value[2][2], default_value[3][2],
      default_value[0][3], default_value[1][3], default_value[2][3], default_value[3][3]};
  // clang-format on
  return GetMat4(GetElement<json::array_t>(&root, path, dv));
}

}  // namespace luka
