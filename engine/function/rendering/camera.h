/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include "core/math.h"

namespace luka {

class Camera {
public:
  Camera();

  void LookAt(const glm::vec4& from, const glm::vec4& to);
private:


};

}  // namespace luka
