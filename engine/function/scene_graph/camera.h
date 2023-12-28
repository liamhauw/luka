// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Camera : public Component {
 public:
  Camera(const std::string& name);
  virtual ~Camera() = default;
  std::type_index GetType() override;
};

class PerspectiveCamera : public Camera {
 public:
  PerspectiveCamera(f32 aspect_ratio, f32 yfov, f32 znear, f32 zfar,
                    const std::string& name);
  virtual ~PerspectiveCamera() = default;

 private:
  f32 aspect_ratio_;
  f32 yfov_;
  f32 znear_;
  f32 zfar_;
};

}  // namespace sg

}  // namespace luka
