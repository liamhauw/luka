// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/buffer.h"
#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Primitive {
 public:
  Primitive() = default;
  ~Primitive() = default;
  Primitive(const Primitive&) = delete;
  Primitive(Primitive&& rhs) noexcept
      : vertex_buffers{std::exchange(rhs.vertex_buffers, {})} {}
  Primitive& operator=(const Primitive&) = delete;
  Primitive& operator=(Primitive&& rhs) noexcept {
    if (this != &rhs) {
      std::swap(vertex_buffers, rhs.vertex_buffers);
    }
    return *this;
  }

  std::map<std::string, gpu::Buffer> vertex_buffers;
};

class Mesh : public Component {
 public:
  Mesh(std::vector<Primitive>&& primitives, const std::string& name = {});
  virtual ~Mesh() = default;
  std::type_index GetType() override;

 private:
  std::vector<Primitive> primitives_;
};

}  // namespace sg

}  // namespace luka
