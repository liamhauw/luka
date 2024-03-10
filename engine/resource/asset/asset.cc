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
  const std::vector<cfg::Scene> cfg_scenes{config_->GetScenes()};
  const std::vector<cfg::Shader> cfg_shaders{config_->GetShaders()};

  for (const auto& cfg_scene : cfg_scenes) {
    ast::Scene scene{gpu_, cfg_scene};
    scenes_.push_back(std::move(scene));
  }

  for (const auto& cfg_shader : cfg_shaders) {
    ast::Shader shader{cfg_shader};
    shaders_.push_back(std::move(shader));
  }
}

void Asset::Tick() {}

const ast::Scene& Asset::GetScene(u32 index) const {
  if (index >= scenes_.size()) {
    THROW("Fail to get scene");
  }
  return scenes_[index];
}

const ast::Shader& Asset::GetShader(u32 index) const {
  if (index >= shaders_.size()) {
    THROW("Fail to get shader");
  }
  return shaders_[index];
}

}  // namespace luka
