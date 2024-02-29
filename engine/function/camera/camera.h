// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/math.h"
#include "function/window/window.h"

namespace luka {

class Camera {
 public:
  Camera(std::shared_ptr<Window> window);

  void Tick();

  void Move(const glm::vec3& camera_relative_pos);

  void Rotate(f32 yaw, f32 pitch);

  const glm::vec3& GetPosition() const;
  const glm::mat4& GetViewMatrix();
  const glm::mat4& GetProjectionMatrix() const;

 private:
  std::shared_ptr<Window> window_;

  glm::vec3 position_{0.0F, 5.0F, -10.0F};
  glm::vec3 look_{0.0F, 0.0F, 1.0F};
  glm::vec3 right_{1.0F, 0.0F, 0.0F};

  bool view_matrix_dirty_{false};
  glm::mat4 view_matirx_;
  glm::mat4 projection_matirx_;

  glm::quat rotation_{glm::identity<glm::quat>()};
};

}  // namespace luka
