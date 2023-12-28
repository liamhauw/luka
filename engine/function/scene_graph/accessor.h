// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/component.h"


namespace luka {

namespace sg {

class BufferView;

class Accessor : public Component {
 public:
  Accessor(BufferView* buffer_view, u64 byte_offset, u64 count);
  virtual ~Accessor() = default;
  std::type_index GetType() override;

 private:
  BufferView* buffer_view_;
  u64 byte_offset_;
  u64 count_;
};

}  // namespace sg

}  // namespace luka