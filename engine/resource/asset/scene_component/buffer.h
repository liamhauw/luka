// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

#include "core/util.h"
#include "resource/asset/scene_component/component.h"

namespace luka::ast::sc {

class Buffer : public Component {
 public:
  explicit Buffer(const std::vector<u8>* data, const std::string& name = {});

  explicit Buffer(const tinygltf::Buffer& tinygltf_buffer);

  ~Buffer() override = default;

  DELETE_SPECIAL_MEMBER_FUNCTIONS(Buffer)

  std::type_index GetType() override;

  const std::vector<u8>* GetData() const;

 private:
  const std::vector<u8>* data_;
};

}  // namespace luka::ast::sc
