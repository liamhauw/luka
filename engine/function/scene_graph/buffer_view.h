// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"

namespace luka {

namespace sg {

class Buffer;

class BufferView : public Component {
 public:
  BufferView(Buffer* buffer, u64 byte_offset, u64 byte_length, u64 byte_stride,
             const std::string& name);
  virtual ~BufferView() = default;
  std::type_index GetType() override;

 private:
  Buffer* buffer_;
  u64 byte_offset_;
  u64 byte_length_;
  u64 byte_stride_;
};

}  // namespace sg

}  // namespace luka