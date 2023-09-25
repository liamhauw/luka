/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Engine source file.
*/

#include "engine.h"

#include "context.h"

namespace luka {

void Engine::Run() {
  gContext.config = std::make_shared<Config>();
  gContext.asset = std::make_shared<Asset>();
  gContext.physics = std::make_shared<Physics>();
  gContext.world = std::make_shared<World>();
  gContext.window = std::make_shared<Window>();
  gContext.function_input = std::make_shared<FunctionInput>();
  gContext.rendering = std::make_shared<Rendering>();
  gContext.editor_input = std::make_shared<EditorInput>();
  gContext.scene = std::make_shared<Scene>();
  gContext.ui = std::make_shared<UI>();

  while (true) {
    gContext.config->Tick();
    gContext.asset->Tick();
    gContext.physics->Tick();
    gContext.world->Tick();
    if (!gContext.window->Tick()) {
      break;
    }
    gContext.function_input->Tick();
    gContext.rendering->Tick();
    gContext.editor_input->Tick();
    gContext.scene->Tick();
    gContext.ui->Tick();
  }
}

}  // namespace luka
