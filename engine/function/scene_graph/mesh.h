// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka
{
  
namespace sg
{

struct Primitive {

};
  
class Mesh : public Component {
public:


private:
  std::vector<Primitive> primitives_;
};

} // namespace sg


} // namespace luka
