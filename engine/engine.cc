// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "engine.h"

namespace luka {

Engine::Engine()
    : config_{std::make_shared<Config>()},
      task_scheduler_{std::make_shared<TaskScheduler>()},
      time_{std::make_shared<Time>()},
      window_{std::make_shared<Window>(time_)},
      gpu_{std::make_shared<Gpu>(window_)},
      asset_{std::make_shared<Asset>(config_, task_scheduler_, gpu_)},
      function_input_{std::make_shared<FunctionInput>(config_, window_)},
      camera_{std::make_shared<Camera>(window_)},
      function_ui_{std::make_shared<FunctionUi>()},
      editor_input_{
          std::make_shared<EditorInput>(config_, time_, window_, camera_)},
      editor_ui_{std::make_shared<EditorUi>()},
      compute_{std::make_shared<Compute>()},
      graphics_{std::make_shared<Graphics>(window_, gpu_, asset_, camera_,
                                           function_ui_)} {}

void Engine::Run() {
  while (!window_->WindowShouldClose()) {
    config_->Tick();
    task_scheduler_->Tick();
    time_->Tick();
    window_->Tick();
    gpu_->Tick();
    asset_->Tick();
    function_input_->Tick();
    camera_->Tick();
    function_ui_->Tick();
    editor_input_->Tick();
    editor_ui_->Tick();
    compute_->Tick();
    graphics_->Tick();
  }
}

}  // namespace luka
