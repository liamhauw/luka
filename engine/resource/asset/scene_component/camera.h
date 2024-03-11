// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/math.h"
#include "resource/asset/scene_component/component.h"

namespace luka {

namespace ast::sc {

enum class CameraType { kNone = -1, kPerspective, kCount };

class Camera : public Component {
 public:
  Camera(CameraType type, f32 aspect_ratioF, f32 yfov, f32 znear, f32 zfar,
         const std::string& name = {});
  Camera(const tinygltf::Camera& tinygltf_camera);

  virtual ~Camera() = default;
  std::type_index GetType() override;

 private:
  CameraType type_;
  f32 aspect_ratio_;
  f32 yfov_;
  f32 znear_;
  f32 zfar_;
};

}  // namespace ast::sc

}  // namespace luka
