// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/camera.h"

#include "core/log.h"

namespace luka {

namespace ast::sc {

Camera::Camera(CameraType type, f32 aspect_ratio, f32 yfov, f32 znear, f32 zfar,
               const std::string& name)
    : Component{name},
      type_{type},
      aspect_ratio_{aspect_ratio},
      yfov_{yfov},
      znear_{znear},
      zfar_{zfar} {}

Camera::Camera(const tinygltf::Camera& model_camera)
    : Component{model_camera.name} {
  const std::string& type{model_camera.type};
  if (type == "perspective") {
    type_ = CameraType::kPerspective;
    const tinygltf::PerspectiveCamera& perspective{model_camera.perspective};
    aspect_ratio_ = static_cast<f32>(perspective.aspectRatio);
    yfov_ = static_cast<f32>(perspective.yfov);
    znear_ = static_cast<f32>(perspective.znear);
    zfar_ = static_cast<f32>(perspective.zfar);
  } else {
    THROW("Unsupport camera type.");
  }
}

std::type_index Camera::GetType() { return typeid(Camera); }

}  // namespace ast::sc

}  // namespace luka
