/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Engine source file.
*/

#include "engine.h"

#include <chrono>
#include <memory>
#include <thread>

#include "context.h"
namespace luka {

void Engine::Run() {
  gContext.is_editor_mode = false;
  gContext.config = std::make_shared<Config>();
  gContext.asset = std::make_shared<Asset>();
  gContext.time = std::make_shared<Time>();
  gContext.window = std::make_shared<Window>();
  gContext.function_input = std::make_shared<FunctionInput>();
  gContext.physics = std::make_shared<Physics>();
  gContext.world = std::make_shared<World>();
  gContext.rendering = std::make_shared<Rendering>();
  gContext.editor_input = std::make_shared<EditorInput>();
  gContext.scene = std::make_shared<Scene>();
  gContext.ui = std::make_shared<UI>();

  while (!gContext.window->WindowShouldClose()) {
    gContext.config->Tick();
    gContext.asset->Tick();
    gContext.time->Tick();
    gContext.window->Tick();
    gContext.function_input->Tick();
    gContext.physics->Tick();
    gContext.world->Tick();
    gContext.rendering->Tick();
    gContext.editor_input->Tick();
    gContext.scene->Tick();
    gContext.ui->Tick();

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  gContext.rendering->~Rendering();
}

}  // namespace luka
