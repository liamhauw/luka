// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Scene : public Component {
 public:
  Scene(const std::string& name ={});
  virtual ~Scene() = default;
  std::type_index GetType() override;

 private:
};

}  // namespace sg

}  // namespace luka
