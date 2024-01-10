// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "engine.h"

#include "context.h"

namespace luka {

void Engine::Run() {
  gContext.load = true;
  gContext.is_editor_mode = false;
  gContext.config = std::make_shared<Config>();
  gContext.asset = std::make_shared<Asset>();
  gContext.time = std::make_shared<Time>();
  gContext.window = std::make_shared<Window>();
  gContext.function_input = std::make_shared<FunctionInput>();
  gContext.physics = std::make_shared<Physics>();
  gContext.world = std::make_shared<World>();
  gContext.camera = std::make_shared<Camera>();
  gContext.gpu = std::make_shared<Gpu>();
  gContext.scene_graph = std::make_shared<SceneGraph>();
  gContext.rendering = std::make_shared<Rendering>();
  // gContext.function_ui = std::make_shared<FunctionUi>();
  gContext.editor_input = std::make_shared<EditorInput>();
  gContext.scene = std::make_shared<Scene>();
  gContext.editor_ui = std::make_shared<EditorUi>();

  while (!gContext.window->WindowShouldClose()) {
    gContext.config->Tick();
    gContext.asset->Tick();
    gContext.time->Tick();
    gContext.window->Tick();
    gContext.function_input->Tick();
    gContext.physics->Tick();
    gContext.world->Tick();
    gContext.camera->Tick();
    gContext.gpu->Tick();
    gContext.scene_graph->Tick();
    gContext.rendering->Tick();
    // gContext.function_ui->Tick();
    gContext.editor_input->Tick();
    gContext.scene->Tick();
    gContext.editor_ui->Tick();
    if (gContext.load) {
      gContext.load = false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}

}  // namespace luka
