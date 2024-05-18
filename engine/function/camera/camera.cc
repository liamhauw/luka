// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/camera/camera.h"

namespace luka {

Camera::Camera(std::shared_ptr<Window> window)
    : window_{std::move(window)},
      view_matirx_{glm::mat4(glm::lookAt(position_, position_ + look_,
                                         glm::cross(right_, look_)))},
      projection_matirx_{glm::perspective(
          glm::radians(60.0F), window_->GetWindowRatio(), 0.1F, 1000.0F)} {
  projection_matirx_[1][1] *= -1;
}

void Camera::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    projection_matirx_ = glm::perspective(
        glm::radians(60.0F), window_->GetWindowRatio(), 0.1F, 1000.0F);
    projection_matirx_[1][1] *= -1;
  }
}

void Camera::Move(const glm::vec3& camera_relative_pos) {
  position_ += (rotation_ * camera_relative_pos);
  view_matrix_dirty_ = true;
}

void Camera::Rotate(f32 yaw, f32 pitch) {
  glm::quat yaw_quat{glm::angleAxis(yaw, glm::vec3{0.0F, 1.0F, 0.0F})};
  glm::quat pitch_quat{glm::angleAxis(pitch, right_)};
  glm::quat cur_qut{yaw_quat * pitch_quat};

  look_ = cur_qut * look_;
  right_ = cur_qut * right_;

  view_matrix_dirty_ = true;

  rotation_ = cur_qut * rotation_;
}

const glm::vec3& Camera::GetPosition() const { return position_; }

const glm::mat4& Camera::GetViewMatrix() {
  if (view_matrix_dirty_) {
    view_matirx_ = glm::mat4(
        glm::lookAt(position_, position_ + look_, glm::cross(right_, look_)));
    view_matrix_dirty_ = false;
  }
  return view_matirx_;
}

const glm::mat4& Camera::GetProjectionMatrix() const {
  return projection_matirx_;
}

}  // namespace luka