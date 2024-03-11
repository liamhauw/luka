// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

namespace luka {

AssetAsync::AssetAsync(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu)
    : config_{config}, gpu_{gpu} {}

void AssetAsync::Load() {
  const std::vector<cfg::Scene>& cfg_scenes{config_->GetScenes()};
  const std::vector<cfg::Shader>& cfg_shaders{config_->GetShaders()};

  for (const auto& cfg_scene : cfg_scenes) {
    scenes_.emplace_back(gpu_, cfg_scene);
  }

  for (const auto& cfg_shader : cfg_shaders) {
    shaders_.emplace_back(cfg_shader);
  }
}

const ast::Shader& AssetAsync::GetShader(u32 index) {
  if (index >= shaders_.size()) {
    THROW("Fail to get shader");
  }
  return shaders_[index];
}

const ast::Shader& AssetAsync::GetShader(u32 index) {
  if (index >= shaders_.size()) {
    THROW("Fail to get shader");
  }
  return shaders_[index];
}

AssetAsyncLoadTaskSet::AssetAsyncLoadTaskSet(AssetAsync* asset_async)
    : asset_async_{asset_async} {}

void AssetAsyncLoadTaskSet::ExecuteRange(enki::TaskSetPartition range,
                                         uint32_t thread_num) {
  asset_async_->Load();
}

Asset::Asset(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu)
    : config_{config},
      gpu_{gpu},
      asset_async_{config_, gpu_},
      asset_async_load_task_set_{&asset_async_} {
  enki::TaskSchedulerConfig task_scheduler_config;
  task_scheduler_.Initialize(task_scheduler_config);
  task_scheduler_.AddTaskSetToPipe(&asset_async_load_task_set_);
}

void Asset::Tick() {}

const ast::Scene& Asset::GetScene(u32 index) {
  task_scheduler_.WaitforTask(&asset_async_load_task_set_);
  return asset_async_.GetScene(index);
}

const ast::Shader& Asset::GetShader(u32 index) {
  task_scheduler_.WaitforTask(&asset_async_load_task_set_);
  return asset_async_.GetShader(index);
}

}  // namespace luka
