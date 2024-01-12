// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "function/scene_graph/component.h"
#include "function/scene_graph/node.h"

namespace luka {

namespace sg {

class Scene : public Component {
 public:
  Scene(std::vector<Node*>&& nodes, const std::string& name = {});
  
  Scene(const std::vector<Node*> node_components,
        const tinygltf::Scene& model_scene);

  virtual ~Scene() = default;
  std::type_index GetType() override;

  const std::vector<Node*>& GetNodes() const;

 private:
  std::vector<Node*> nodes_;
};

}  // namespace sg

}  // namespace luka
