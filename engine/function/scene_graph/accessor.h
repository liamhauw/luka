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

class BufferView;

class Accessor : public Component {
 public:
  Accessor(BufferView* buffer_view, u64 byte_offset, bool normalized,
           u32 component_type, u64 count, u32 type,
           const std::string& name = {});
  virtual ~Accessor() = default;
  std::type_index GetType() override;


  u64 GetCount() const;
  std::pair<const u8*, u64> GetBuffer() const;
  vk::Format GetFormat() const;
  u32 GetStride() const;

 private:
  u32 GetByteStride(u32 buffer_view_byte_stride);

  BufferView* buffer_view_;
  u64 byte_offset_;
  bool normalized_;
  u32 component_type_;
  u64 count_;
  u32 type_;

  u32 buffer_stride_;
  const u8* buffer_data_;
  u64 buffer_size_;
};

}  // namespace sg

}  // namespace luka