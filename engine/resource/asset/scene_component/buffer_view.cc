// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/buffer_view.h"

namespace luka::ast::sc {

BufferView::BufferView(const Buffer* buffer, u64 byte_offset, u64 byte_length,
                       u64 byte_stride, const std::string& name)
    : Component{name},
      buffer_{buffer},
      byte_offset_{byte_offset},
      byte_length_{byte_length},
      byte_stride_{byte_stride} {}

BufferView::BufferView(const std::vector<ast::sc::Buffer*>& buffer_components,
                       const tinygltf::BufferView& tinygltf_buffer_view)
    : Component{tinygltf_buffer_view.name},
      buffer_{buffer_components[tinygltf_buffer_view.buffer]},
      byte_offset_{tinygltf_buffer_view.byteOffset},
      byte_length_{tinygltf_buffer_view.byteLength},
      byte_stride_{tinygltf_buffer_view.byteStride} {}

std::type_index BufferView::GetType() { return typeid(BufferView); }

const Buffer* BufferView::GetBuffer() const { return buffer_; }

u64 BufferView::GetByteOffset() const { return byte_offset_; }

u64 BufferView::GetByteLength() const { return byte_length_; }

u64 BufferView::GetByteStride() const { return byte_stride_; }

}  // namespace luka::ast::sc
