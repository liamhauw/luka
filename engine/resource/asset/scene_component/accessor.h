// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/asset/scene_component/buffer_view.h"
#include "resource/asset/scene_component/component.h"
#include "resource/gpu/buffer.h"

namespace luka {

namespace ast::sc {

class Accessor : public Component {
 public:
  Accessor(BufferView* buffer_view, u64 byte_offset, bool normalized,
           u32 component_type, u64 count, u32 type,
           const std::string& name = {});

  Accessor(const std::vector<BufferView*>& buffer_view_components,
           const tinygltf::Accessor& tinygltf_accessor);

  virtual ~Accessor() = default;
  std::type_index GetType() override;

  u64 GetCount() const;
  std::pair<const u8*, u64> GetBuffer() const;
  u32 GetStride() const;
  vk::Format GetFormat() const;

 private:
  void CalculateBufferData();
  u32 GetByteStride(u32 buffer_view_byte_stride);
  vk::Format ParseFormat();

  BufferView* buffer_view_;
  u64 byte_offset_;
  bool normalized_;
  u32 component_type_;
  u64 count_;
  u32 type_;

  u32 buffer_stride_;
  const u8* buffer_data_;
  u64 buffer_size_;
  vk::Format format_;
};

}  // namespace ast::sc

}  // namespace luka