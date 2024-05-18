// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "engine.h"

namespace luka {

Engine::Engine()
    : task_scheduler_{std::make_shared<TaskScheduler>()},
      window_{std::make_shared<Window>()},
      gpu_{std::make_shared<Gpu>(window_)},
      config_{std::make_shared<Config>()},
      asset_{std::make_shared<Asset>(config_, task_scheduler_, gpu_)},
      time_{std::make_shared<Time>()},
      camera_{std::make_shared<Camera>(window_)},
      function_input_{std::make_shared<FunctionInput>(config_, window_)},
      function_ui_{std::make_shared<FunctionUi>(window_, gpu_)},
      editor_input_{
          std::make_shared<EditorInput>(config_, window_, time_, camera_)},
      editor_ui_{std::make_shared<EditorUi>(config_, window_, time_)},
      framework_{std::make_shared<Framework>(config_, window_, gpu_, asset_,
                                             camera_, function_ui_)} {}

void Engine::Run() {
  while (!window_->WindowShouldClose()) {
    task_scheduler_->Tick();
    window_->Tick();
    gpu_->Tick();
    config_->Tick();
    asset_->Tick();
    time_->Tick();
    camera_->Tick();
    function_input_->Tick();
    function_ui_->Tick();
    editor_input_->Tick();
    editor_ui_->Tick();
    framework_->Tick();
  }
}

}  // namespace luka
