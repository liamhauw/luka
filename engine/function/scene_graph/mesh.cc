// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/mesh.h"

#include "core/log.h"

namespace luka {

namespace sg {

Mesh::Mesh(std::vector<Primitive>&& primitives, const std::string& name)
    : Component{name}, primitives_{std::move(primitives)} {}

Mesh::Mesh(std::shared_ptr<Gpu> gpu,
           const std::vector<Material*> material_components,
           const std::vector<Accessor*> accessor_components,
           const tinygltf::Mesh& model_mesh,
           const vk::raii::CommandBuffer& command_buffer,
           std::vector<gpu::Buffer>& staging_buffers)
    : Component{model_mesh.name} {
  const std::vector<tinygltf::Primitive> model_primitives{
      model_mesh.primitives};

  for (const auto& model_primitive : model_primitives) {
    Primitive primitive;

    // Vertex.
    for (const auto& attribute : model_primitive.attributes) {
      const std::string& attribute_name{attribute.first};
      u32 attribute_accessor_index{static_cast<u32>(attribute.second)};
      Accessor* accessor{accessor_components[attribute_accessor_index]};

      auto accessor_buffer{accessor->GetBuffer()};
      const u8* buffer_data{accessor_buffer.first};
      u64 buffer_size{accessor_buffer.second};

      vk::BufferCreateInfo staging_buffer_ci{
          {}, buffer_size, vk::BufferUsageFlagBits::eTransferSrc};

      vk::BufferCreateInfo buffer_ci{{},
                                     buffer_size,
                                     vk::BufferUsageFlagBits::eIndexBuffer |
                                         vk::BufferUsageFlagBits::eTransferDst,
                                     vk::SharingMode::eExclusive};

      gpu::Buffer staging_buffer{
          gpu->CreateBuffer(staging_buffer_ci, buffer_data)};
      staging_buffers.push_back(std::move(staging_buffer));

      gpu::Buffer buffer{
          gpu->CreateBuffer(buffer_ci, staging_buffers.back(), command_buffer)};
      primitive.vertex_buffers.insert(
          std::make_pair(attribute_name, std::move(buffer)));

      vk::Format format{accessor->GetFormat()};
      u32 stride{accessor->GetStride()};
      VertexAttribute vertex_attribute{format, stride, 0};

      primitive.vertex_attributes.insert(
          std::make_pair(attribute_name, std::move(vertex_attribute)));

      primitive.vertex_count = accessor->GetCount();
    }

    // Index.
    if (model_primitive.indices != -1) {
      primitive.has_index = true;

      u32 attribute_accessor_index{static_cast<u32>(model_primitive.indices)};
      Accessor* accessor{accessor_components[attribute_accessor_index]};

      auto accessor_buffer{accessor->GetBuffer()};
      const u8* buffer_data{accessor_buffer.first};
      u64 buffer_size{accessor_buffer.second};

      vk::Format format{accessor->GetFormat()};

      vk::IndexType index_type;
      switch (format) {
        case vk::Format::eR8Uint:
          index_type = vk::IndexType::eUint8EXT;
          break;
        case vk::Format::eR16Uint:
          index_type = vk::IndexType::eUint16;
          break;
        case vk::Format::eR32Uint:
          index_type = vk::IndexType::eUint32;
          break;
        default:
          THROW("Unsupport format");
          break;
      }

      vk::BufferCreateInfo staging_buffer_ci{
          {}, buffer_size, vk::BufferUsageFlagBits::eTransferSrc};

      vk::BufferCreateInfo buffer_ci{{},
                                     buffer_size,
                                     vk::BufferUsageFlagBits::eIndexBuffer |
                                         vk::BufferUsageFlagBits::eTransferDst,
                                     vk::SharingMode::eExclusive};

      gpu::Buffer staging_buffer{
          gpu->CreateBuffer(staging_buffer_ci, buffer_data)};
      staging_buffers.push_back(std::move(staging_buffer));

      gpu::Buffer buffer{
          gpu->CreateBuffer(buffer_ci, staging_buffers.back(), command_buffer)};

      u64 index_count{accessor->GetCount()};

      IndexAttribute index_attribute{index_type, index_count};

      primitive.index_buffer = std::move(buffer);
      primitive.index_attribute = std::move(index_attribute);
    }

    // Material.
    if (model_primitive.material != -1) {
      primitive.material = material_components[model_primitive.material];
    } else {
      primitive.material = material_components.back();
    }

    primitives_.emplace_back(std::move(primitive));
  }
}

std::type_index Mesh::GetType() { return typeid(Mesh); }

}  // namespace sg

}  // namespace luka
