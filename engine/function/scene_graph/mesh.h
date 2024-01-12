// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "function/gpu/buffer.h"
#include "function/gpu/gpu.h"
#include "function/scene_graph/accessor.h"
#include "function/scene_graph/component.h"
#include "function/scene_graph/material.h"

namespace luka {

namespace sg {

struct VertexAttribute {
  vk::Format format{vk::Format::eUndefined};
  u32 stride{0};
  u32 offset{0};
};

struct IndexAttribute {
  vk::IndexType index_type{vk::IndexType::eUint16};
  u64 count{0};
};

struct Primitive {
 public:
  Primitive() = default;
  ~Primitive() = default;
  Primitive(const Primitive&) = delete;
  Primitive(Primitive&& rhs) = default;
  Primitive& operator=(const Primitive&) = delete;
  Primitive& operator=(Primitive&& rhs) = default;

  std::map<std::string, gpu::Buffer> vertex_buffers;
  gpu::Buffer index_buffer{nullptr};

  std::map<std::string, VertexAttribute> vertex_attributes;
  u64 vertex_count{0};
  bool has_index{false};
  IndexAttribute index_attribute;

  Material* material;
};

class Mesh : public Component {
 public:
  Mesh(std::vector<Primitive>&& primitives, const std::string& name = {});

  Mesh(std::shared_ptr<Gpu> gpu,
       const std::vector<Material*> material_components,
       const std::vector<Accessor*> accessor_components,
       const tinygltf::Mesh& model_mesh,
       const vk::raii::CommandBuffer& command_buffer,
       std::vector<gpu::Buffer>& staging_buffers);

  virtual ~Mesh() = default;
  std::type_index GetType() override;

 private:
  std::vector<Primitive> primitives_;
};

}  // namespace sg

}  // namespace luka
