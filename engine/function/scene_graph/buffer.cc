// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/buffer.h"

namespace luka {

namespace sg {

Buffer::Buffer(const std::vector<u8>* data, const std::string& name)
    : Component{name}, data_{data} {}

Buffer::Buffer(const tinygltf::Buffer& model_buffer)
    : Component{model_buffer.name}, data_{&(model_buffer.data)} {}

std::type_index Buffer::GetType() { return typeid(Buffer); }

const std::vector<u8>* Buffer::GetData() const { return data_; }

}  // namespace sg

}  // namespace luka