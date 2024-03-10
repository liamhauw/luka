// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "resource/asset/scene_component/component.h"

namespace luka {

namespace ast::sc {

class Buffer : public Component {
 public:
  Buffer(const std::vector<u8>* data, const std::string& name = {});

  Buffer(const tinygltf::Buffer& model_buffer);

  virtual ~Buffer() = default;
  std::type_index GetType() override;

  const std::vector<u8>* GetData() const;

 private:
  const std::vector<u8>* data_;
};

}  // namespace ast::sc

}  // namespace luka
