// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "editor/editor_input/editor_input.h"
#include "editor/editor_ui/editor_ui.h"
#include "editor/scene/scene.h"
#include "function/camera/camera.h"
#include "function/function_input/function_input.h"
#include "function/function_ui/function_ui.h"
#include "function/gpu/gpu.h"
#include "function/physics/physics.h"
#include "function/rendering/rendering.h"
#include "function/time/time.h"
#include "function/window/window.h"
#include "function/world/world.h"
#include "resource/asset/asset.h"
#include "resource/config/config.h"

namespace luka {

struct Context {
  bool load;
  bool is_editor_mode;
  std::shared_ptr<Config> config;
  std::shared_ptr<Asset> asset;
  std::shared_ptr<Time> time;
  std::shared_ptr<Window> window;
  std::shared_ptr<FunctionInput> function_input;
  std::shared_ptr<Physics> physics;
  std::shared_ptr<World> world;
  std::shared_ptr<Camera> camera;
  std::shared_ptr<Gpu> gpu;
  std::shared_ptr<FunctionUI> function_ui;
  std::shared_ptr<Rendering> rendering;
  std::shared_ptr<EditorInput> editor_input;
  std::shared_ptr<Scene> scene;
  std::shared_ptr<EditorUI> editor_ui;
};

extern Context gContext;

}  // namespace luka