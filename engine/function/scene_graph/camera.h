// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/math.h"
#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

enum class CameraType { kNone = -1, kPerspective, kCount };

class Camera : public Component {
 public:
  Camera(CameraType type, f32 aspect_ratioF, f32 yfov, f32 znear, f32 zfar,
         const std::string& name = {});
  Camera(const tinygltf::Camera& model_camera);

  virtual ~Camera() = default;
  std::type_index GetType() override;

 private:
  CameraType type_;
  f32 aspect_ratio_;
  f32 yfov_;
  f32 znear_;
  f32 zfar_;
};

}  // namespace sg

}  // namespace luka
