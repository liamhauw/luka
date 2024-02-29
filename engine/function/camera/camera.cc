// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"

namespace luka {

Camera::Camera(std::shared_ptr<Window> window)
    : window_{window},
      view_matirx_{glm::mat4(
          glm::lookAt(position_, position_ + look_, glm::cross(right, look_)))},
      projection_matirx_{glm::perspective(
          glm::radians(60.0F), window_->GetWindowRatio(), 0.1F, 1000.0F)} {}

void Camera::Tick() {
  if (window_->GetFramebufferResized()) {
    projection_matirx_ = glm::perspective(
        glm::radians(60.0F), window_->GetWindowRatio(), 0.1F, 1000.0F);
  }
}

void Camera::Move(const glm::vec3& camera_relative_pos) {
  position_ += camera_relative_pos;
  view_matrix_dirty_ = true;
}

void Camera::Rotate(const glm::vec2& delta) {}

const glm::vec3& Camera::GetPosition() const { return position_; }

const glm::mat4& Camera::GetViewMatrix() {
  if (view_matrix_dirty_) {
    view_matirx_ = glm::mat4(
        glm::lookAt(position_, position_ + look_, glm::cross(right, look_)));
    view_matrix_dirty_ = false;
  }
  return view_matirx_;
}

const glm::mat4& Camera::GetProjectionMatrix() const {
  return projection_matirx_;
}

}  // namespace luka