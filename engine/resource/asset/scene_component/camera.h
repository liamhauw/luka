// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/math.h"
#include "core/util.h"
#include "resource/asset/scene_component/component.h"

namespace luka::ast::sc {

enum class CameraType { kNone = -1, kPerspective, kCount };

class Camera : public Component {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Camera)

  Camera(CameraType type, f32 aspect_ratio, f32 yfov, f32 znear, f32 zfar,
         const std::string& name = {});
  explicit Camera(const tinygltf::Camera& tinygltf_camera);

  ~Camera() override = default;

  std::type_index GetType() override;

 private:
  CameraType type_;
  f32 aspect_ratio_;
  f32 yfov_;
  f32 znear_;
  f32 zfar_;
};

}  // namespace luka::ast::sc
