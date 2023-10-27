/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "core/math.h"

NLOHMANN_JSON_NAMESPACE_BEGIN

template <>
struct adl_serializer<glm::vec4> {
  static void from_json(const json& j, glm::vec4& v) {
    v = {j[0], j[1], j[2], j.size() == 4 ? j[3] : 0};
  }
};

template <>
struct adl_serializer<glm::mat4> {
  static void from_json(const json& j, glm::mat4& m) {
    m = glm::mat4(glm::vec4(j[0], j[1], j[2], j[3]),
                  glm::vec4(j[4], j[5], j[6], j[7]),
                  glm::vec4(j[8], j[9], j[10], j[11]),
                  glm::vec4(j[12], j[13], j[14], j[15]));
  }
};

NLOHMANN_JSON_NAMESPACE_END
