// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/util.h"
#include "resource/asset/scene_component/component.h"
#include "resource/asset/scene_component/node.h"

namespace luka::ast::sc {

class Scene : public Component {
 public:
  explicit Scene(std::vector<Node*>&& nodes, const std::string& name = {});

  Scene(const std::vector<Node*>& node_components,
        const tinygltf::Scene& tinygltf_scene);

  ~Scene() override = default;

  DELETE_SPECIAL_MEMBER_FUNCTIONS(Scene)

  std::type_index GetType() override;

  const std::vector<Node*>& GetNodes() const;

 private:
  std::vector<Node*> nodes_;
};

}  // namespace luka::ast::sc
