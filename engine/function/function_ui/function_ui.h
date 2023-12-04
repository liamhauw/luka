/*
  SPDX license identifier: MIT.
  Copyright (C) 2023 Liam Hauw.
*/

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

class Gpu;

class FunctionUi {
 public:
  FunctionUi();
  ~FunctionUi();

  void Tick();

 private:
  void Resize();
  void Render(const vk::raii::CommandBuffer& command_buffer);

  void CreateImgui();
  void AddViewportImage();
  void DestroyImgui();
  void CreateUi();

  std::shared_ptr<Gpu> gpu_;

  vk::DescriptorSet descriptor_set_;
};

}  // namespace luka
