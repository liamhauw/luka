// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/util.h"

namespace luka::ast::sc {

class Component {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Component);

  explicit Component(std::string name = {});

  virtual ~Component() = default;

  virtual std::type_index GetType() = 0;
  const std::string& GetName() const;

 private:
  std::string name_;
};

}  // namespace luka::ast::sc
