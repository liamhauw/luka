// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/asset/scene_component/buffer.h"
#include "resource/asset/scene_component/component.h"

namespace luka {

namespace ast::sc {

class BufferView : public Component {
 public:
  BufferView(const Buffer* buffer, u64 byte_offset, u64 byte_length,
             u64 byte_stride, const std::string& name = {});

  BufferView(const std::vector<ast::sc::Buffer*>& buffer_components,
             const tinygltf::BufferView& tinygltf_buffer_view);

  virtual ~BufferView() = default;
  std::type_index GetType() override;

  const Buffer* GetBuffer() const;
  u64 GetByteOffset() const;
  u64 GetByteLength() const;
  u64 GetByteStride() const;

 private:
  const Buffer* buffer_;
  u64 byte_offset_;
  u64 byte_length_;
  u64 byte_stride_;
};

}  // namespace ast::sc

}  // namespace luka