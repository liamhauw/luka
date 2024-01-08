// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Node;

class Scene : public Component {
 public:
  Scene(std::vector<Node*>&& nodes, const std::string& name = {});
  virtual ~Scene() = default;
  std::type_index GetType() override;

  const std::vector<Node*>& GetNodes() const;

 private:
  std::vector<Node*> nodes_;
};

}  // namespace sg

}  // namespace luka
