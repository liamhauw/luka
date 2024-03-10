// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/model.h"
#include "resource/asset/shader.h"
#include "resource/config/config.h"
#include "resource/gpu/gpu.h"

namespace luka {

class Asset {
 public:
  Asset(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu);

  void Tick();

  const ast::Model& GetModel() const;
  const ast::Shader& GetVertexShader() const;
  const ast::Shader& GetFragmentShader() const;

 private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<Gpu> gpu_;

  std::vector<ast::Model> models_;
  std::vector<ast::Shader> shaders_;
};

}  // namespace luka