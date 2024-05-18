// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/gpu/gpu.h"
#include "base/task_scheduler/task_scheduler.h"
#include "base/window/window.h"
#include "editor/editor_input/editor_input.h"
#include "editor/editor_ui/editor_ui.h"
#include "function/camera/camera.h"
#include "function/function_input/function_input.h"
#include "function/function_ui/function_ui.h"
#include "function/time/time.h"
#include "rendering/graphics/graphics.h"
#include "resource/asset/asset.h"
#include "resource/config/config.h"

namespace luka {

class Engine {
 public:
  Engine();

  void Run();

 private:
  std::shared_ptr<TaskScheduler> task_scheduler_;
  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Config> config_;
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Time> time_;
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<FunctionInput> function_input_;
  std::shared_ptr<FunctionUi> function_ui_;
  std::shared_ptr<EditorInput> editor_input_;
  std::shared_ptr<EditorUi> editor_ui_;
  std::shared_ptr<Graphics> graphics_;
};

}  // namespace luka
