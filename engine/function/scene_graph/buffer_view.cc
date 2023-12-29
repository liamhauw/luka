// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/buffer_view.h"

namespace luka {

namespace sg {

BufferView::BufferView(Buffer* buffer, u64 byte_offset, u64 byte_length,
                       u64 byte_stride, const std::string& name)
    : Component{name},
      buffer_{buffer},
      byte_offset_{byte_offset},
      byte_length_{byte_length},
      byte_stride_{byte_stride} {}

std::type_index BufferView::GetType() { return typeid(BufferView); }

Buffer* BufferView::GetBuffer() const { return buffer_; }

u64 BufferView::GetByteOffset() const { return byte_offset_; }

u64 BufferView::GetByteLength() const { return byte_length_; }

u64 BufferView::GetByteStride() const { return byte_stride_; }

}  // namespace sg

}  // namespace luka