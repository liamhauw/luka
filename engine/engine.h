/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Engine header file.
*/

#pragma once

#include <memory>

namespace luka {

class Config;
class Asset;
class Physics;
class World;
class Window;
class Input;
class Rendering;
class EditorInput;
class Scene;
class UI;

struct Context {
  std::shared_ptr<Config> config;
  std::shared_ptr<Asset> asset;
  std::shared_ptr<Physics> physics;
  std::shared_ptr<World> world;
  std::shared_ptr<Window> window;
  std::shared_ptr<Input> input;
  std::shared_ptr<Rendering> rendering;
  std::shared_ptr<EditorInput> editor_input;
  std::shared_ptr<Scene> scene;
  std::shared_ptr<UI> ui;
};

extern Context gContext;

class Engine {
 public:
  static void Run();
};

}  // namespace luka
