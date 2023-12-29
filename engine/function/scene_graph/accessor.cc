// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/accessor.h"

#include "core/log.h"
#include "function/scene_graph/buffer.h"
#include "function/scene_graph/buffer_view.h"
#include "resource/asset/model.h"

namespace luka {

namespace sg {

Accessor::Accessor(BufferView* buffer_view, u64 byte_offset, bool normalized,
                   u32 component_type, u64 count, u32 type,
                   const std::string& name)
    : Component{name},
      buffer_view_{buffer_view},
      byte_offset_{byte_offset},
      normalized_{normalized},
      component_type_{component_type},
      count_{count},
      type_{type} {
  const u8* whole_data_begin{buffer_view_->GetBuffer()->GetData()->data()};
  buffer_data_ =
      whole_data_begin + byte_offset_ + buffer_view_->GetByteOffset();
  buffer_size_ = count_ * GetByteStride(buffer_view_->GetByteStride());
}

std::type_index Accessor::GetType() { return typeid(Accessor); }

std::pair<const u8*, u64> Accessor::GetBuffer() const {
  return std::make_pair(buffer_data_, buffer_size_);
}

u32 Accessor::GetByteStride(u32 buffer_view_byte_stride) {
  if (buffer_view_byte_stride == 0) {
    i32 component_size_in_byte{
        tinygltf::GetComponentSizeInBytes(component_type_)};
    if (component_size_in_byte <= 0) {
      THROW("Unsupport component type");
    }

    i32 component_count{tinygltf::GetNumComponentsInType(type_)};
    if (component_count <= 0) {
      THROW("Unsupport type");
    }

    return static_cast<u32>(component_size_in_byte * component_count);
  } else {
    i32 component_size_in_byte{
        tinygltf::GetComponentSizeInBytes(component_type_)};
    if (component_size_in_byte <= 0) {
      THROW("Unsupport component type");
    }

    if (buffer_view_byte_stride % static_cast<u32>(component_size_in_byte) !=
        0) {
      THROW("Unsupport component type");
    }
    return static_cast<u32>(buffer_view_byte_stride);
  }
}

}  // namespace sg

}  // namespace luka