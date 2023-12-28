// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/accessor.h"

namespace luka {

namespace sg {

Accessor::Accessor(BufferView* buffer_view, u64 byte_offset, u64 count)
    : buffer_view_{buffer_view}, byte_offset_{byte_offset}, count_{count} {}

std::type_index Accessor::GetType() { return typeid(Accessor); }

}  // namespace sg

}  // namespace luka