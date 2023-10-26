/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "core/math.h"

namespace luka {

glm::vec4 GetVec4(const json::array_t& json_array);

glm::mat4 GetMat4(const json::array_t& json_array);

std::string GetElementString(const json::object_t& root,
                             const std::string& path,
                             const std::string& default_value);

uint32_t GetElementUint32(const json::object_t& root, const std::string& path,
                          uint32_t default_value);

json::array_t GetElementJsonArray(const json::object_t& root,
                                  const std::string& path,
                                  const json::array_t& default_value);

glm::vec4 GetElementVec4(const json::object_t& root, const std::string& path,
                         const glm::vec4& default_value);

glm::mat4 GetElementMat4(const json::object_t& root, const std::string& path,
                         const glm::mat4& default_value);

}  // namespace luka
