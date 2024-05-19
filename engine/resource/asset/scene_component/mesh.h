// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "base/gpu/buffer.h"
#include "base/gpu/gpu.h"
#include "core/util.h"
#include "resource/asset/scene_component/accessor.h"
#include "resource/asset/scene_component/component.h"
#include "resource/asset/scene_component/material.h"

namespace luka::ast::sc {

struct VertexAttribute {
  gpu::Buffer buffer;
  vk::Format format;
  u32 stride;
  u32 offset;
  u64 count;
  u32 location;
};

struct IndexAttribute {
  gpu::Buffer buffer;
  vk::IndexType index_type;
  u64 offset;
  u64 count;
};

class Primitive {
 public:
  Primitive() = default;
  Primitive(const Primitive&) = delete;
  Primitive(Primitive&& rhs) noexcept = default;

  ~Primitive() = default;

  Primitive& operator=(const Primitive&) = delete;
  Primitive& operator=(Primitive&& rhs) = default;

  std::map<std::string, VertexAttribute> vertex_attributes;
  IndexAttribute index_attribute;
  bool has_index{};
  const Material* material{};
};

class Mesh : public Component {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Mesh)

  explicit Mesh(std::vector<Primitive>&& primitives,
                const std::string& name = {});
  Mesh(const std::shared_ptr<Gpu>& gpu,
       const std::vector<Material*>& material_components,
       const std::vector<Accessor*>& accessor_components,
       const tinygltf::Mesh& tinygltf_mesh,
       const vk::raii::CommandBuffer& command_buffer,
       std::vector<gpu::Buffer>& staging_buffers);

  ~Mesh() override = default;

  std::type_index GetType() override;

  const std::vector<Primitive>& GetPrimitives() const;

 private:
  std::vector<Primitive> primitives_;
};

}  // namespace luka::ast::sc
