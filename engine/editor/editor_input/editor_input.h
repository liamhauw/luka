/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Editor input header file.
*/

#pragma once

namespace luka {

class EditorInput {
 public:
  EditorInput();

  void Tick();

  static void OnKey(int key, int scancode, int action, int mod);
};

}  // namespace luka
