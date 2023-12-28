// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/camera.h"

namespace luka {

namespace sg {

Camera::Camera(const std::string& name) : Component{name} {}

std::type_index Camera::GetType() { return typeid(Camera); }

PerspectiveCamera::PerspectiveCamera(f32 aspect_ratio, f32 yfov, f32 znear,
                                     f32 zfar, const std::string& name)

    : Camera{name},
      aspect_ratio_{aspect_ratio},
      yfov_{yfov},
      znear_{znear},
      zfar_{zfar} {}

}  // namespace sg

}  // namespace luka
