/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Engine source file.
*/

#include "engine.h"

#include "editor/input/editor_input.h"
#include "editor/scene/scene.h"
#include "editor/ui/ui.h"
#include "function/input/input.h"
#include "function/physics/physics.h"
#include "function/rendering/rendering.h"
#include "function/window/window.h"
#include "function/world/world.h"
#include "resource/asset/asset.h"
#include "resource/config/config.h"

namespace luka {

Context gContext;

void Engine::Run() {
  gContext.config = std::make_shared<Config>();
  gContext.asset = std::make_shared<Asset>();
  gContext.physics = std::make_shared<Physics>();
  gContext.world = std::make_shared<World>();
  gContext.window = std::make_shared<Window>();
  gContext.input = std::make_shared<Input>();
  gContext.rendering = std::make_shared<Rendering>();
  gContext.editor_input = std::make_shared<EditorInput>();
  gContext.scene = std::make_shared<Scene>();
  gContext.ui = std::make_shared<UI>();

  while (true) {
    gContext.scene->Tick();
    gContext.editor_input->Tick();
    gContext.world->Tick();
    gContext.input->Tick();
    gContext.rendering->Tick();
    if (!gContext.window->Tick()) {
      break;
    }
  }
}

}  // namespace luka
