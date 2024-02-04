// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "function/scene_graph/buffer.h"
#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class BufferView : public Component {
 public:
  BufferView(Buffer* buffer, u64 byte_offset, u64 byte_length, u64 byte_stride,
             const std::string& name = {});

  BufferView(const std::vector<sg::Buffer*>& buffer_components,
             const tinygltf::BufferView& model_buffer_view);

  virtual ~BufferView() = default;
  std::type_index GetType() override;

  Buffer* GetBuffer() const;
  u64 GetByteOffset() const;
  u64 GetByteLength() const;
  u64 GetByteStride() const;

 private:
  Buffer* buffer_;
  u64 byte_offset_;
  u64 byte_length_;
  u64 byte_stride_;
};

}  // namespace sg

}  // namespace luka