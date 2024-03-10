// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include "core/log.h"

namespace luka {

Asset::Asset(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu)
    : config_{config}, gpu_{gpu} {
  const std::vector<cfg::Model> cfg_models{config_->GetModels()};
  const std::vector<cfg::Shader> cfg_shaders{config_->GetShaders()};

  for (const auto& cfg_model : cfg_models) {
    ast::Model model{ast::Model{cfg_model}};
  }

  for (const auto& cfg_shader : cfg_shaders) {
    ast::Shader shader{ast::Shader{cfg_shader}};
  }
}

void Asset::Tick() {}

const ast::Model& Asset::GetModel() const { return models_[0]; }

const ast::Shader& Asset::GetVertexShader() const { return shaders_[0]; }

const ast::Shader& Asset::GetFragmentShader() const { return shaders_[1]; }

}  // namespace luka
