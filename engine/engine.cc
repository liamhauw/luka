// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "engine.h"

namespace luka {

Engine::Engine()
    : config_{std::make_shared<Config>()},
      asset_{std::make_shared<Asset>(config_)},
      context_{std::make_shared<Context>(false, true)},
      time_{std::make_shared<Time>()},
      window_{std::make_shared<Window>(time_)},
      function_input_{std::make_shared<FunctionInput>(context_, window_)},
      physics_{std::make_shared<Physics>()},
      world_{std::make_shared<World>()},
      camera_{std::make_shared<Camera>()},
      gpu_{std::make_shared<Gpu>(window_)},
      scene_graph_{std::make_shared<SceneGraph>(asset_, gpu_)},
      rendering_{std::make_shared<Rendering>(window_, gpu_)},
      function_ui_{std::make_shared<FunctionUi>()},
      editor_input_{std::make_shared<EditorInput>(context_, window_)},
      scene_{std::make_shared<Scene>()},
      editor_ui_{std::make_shared<EditorUi>()} {}

void Engine::Run() {
  while (!window_->WindowShouldClose()) {
    config_->Tick();
    asset_->Tick();
    context_->Tick();
    time_->Tick();
    window_->Tick();
    function_input_->Tick();
    physics_->Tick();
    world_->Tick();
    camera_->Tick();
    gpu_->Tick();
    scene_graph_->Tick();
    rendering_->Tick();
    function_ui_->Tick();
    editor_input_->Tick();
    scene_->Tick();
    editor_ui_->Tick();

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}

}  // namespace luka
