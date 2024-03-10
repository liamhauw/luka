// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/buffer.h"

namespace luka {

namespace ast::sc {

Buffer::Buffer(const std::vector<u8>* data, const std::string& name)
    : Component{name}, data_{data} {}

Buffer::Buffer(const tinygltf::Buffer& model_buffer)
    : Component{!model_buffer.name.empty() ? model_buffer.name
                                           : model_buffer.uri},
      data_{&(model_buffer.data)} {}

std::type_index Buffer::GetType() { return typeid(Buffer); }

const std::vector<u8>* Buffer::GetData() const { return data_; }

}  // namespace ast::sc

}  // namespace luka