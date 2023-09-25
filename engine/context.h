/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Context header file,
*/

#pragma once

#include <memory>

#include "editor/editor_input/editor_input.h"
#include "editor/scene/scene.h"
#include "editor/ui/ui.h"
#include "function/function_input/function_input.h"
#include "function/physics/physics.h"
#include "function/rendering/rendering.h"
#include "function/window/window.h"
#include "function/world/world.h"
#include "resource/asset/asset.h"
#include "resource/config/config.h"

namespace luka {

struct Context {
  std::shared_ptr<Config> config;
  std::shared_ptr<Asset> asset;
  std::shared_ptr<Physics> physics;
  std::shared_ptr<World> world;
  std::shared_ptr<Window> window;
  std::shared_ptr<FunctionInput> function_input;
  std::shared_ptr<Rendering> rendering;
  std::shared_ptr<EditorInput> editor_input;
  std::shared_ptr<Scene> scene;
  std::shared_ptr<UI> ui;
};

extern Context gContext;

}  // namespace luka