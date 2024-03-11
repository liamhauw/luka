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
    scenes_.emplace_back(gpu_, cfg_scene);
  }

  for (const auto& cfg_shader : cfg_shaders) {
    shaders_.emplace_back(cfg_shader);
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
