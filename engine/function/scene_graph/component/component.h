// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

namespace sg {

class Component {
 public:
  Component(const std::string& name = {});
  virtual ~Component() = default;
  virtual std::type_index GetType() = 0;
  const std::string& GetName() const;

 private:
  std::string name_;
};

}  // namespace sg

}  // namespace luka
