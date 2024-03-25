/*
  SPDX license identifier: MIT.
  Copyright (C) 2023 Liam Hauw.
*/

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/gpu/gpu.h"
#include "resource/window/window.h"

namespace luka {

class Gpu;

class FunctionUi {
 public:
  FunctionUi(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu);

  ~FunctionUi();

  void Tick();
  void Render(const vk::raii::CommandBuffer& command_buffer);

 private:
  void CreateImgui();
  void DestroyImgui();
  void CreateUi();

  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;

  vk::DescriptorSet descriptor_set_;
};

}  // namespace luka
