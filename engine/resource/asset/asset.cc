// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

namespace luka {

AssetAsync::AssetAsync(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu)
    : config_{config},
      gpu_{gpu},
      cfg_scenes_{&(config_->GetScenes())},
      cfg_shaders_{&(config_->GetShaders())},
      scene_size_{static_cast<u32>(cfg_scenes_->size())},
      shader_size_{static_cast<u32>(cfg_shaders_->size())},
      set_size_{scene_size_ + shader_size_},
      scenes_(scene_size_),
      shaders_(shader_size_) {}

u32 AssetAsync::GetSceneSize() const { return scene_size_; }

u32 AssetAsync::GetShaderSize() const { return shader_size_; }

u32 AssetAsync::GetSetSize() const { return set_size_; }

void AssetAsync::LoadScene(u32 index) {
  scenes_[index] = std::move(ast::Scene{gpu_, (*cfg_scenes_)[index]});
}

void AssetAsync::LoadShader(u32 index) {
  shaders_[index] = std::move(ast::Shader{(*cfg_shaders_)[index]});
}

const ast::Scene& AssetAsync::GetScene(u32 index) {
  if (index >= scene_size_) {
    THROW("Fail to get shader");
  }
  return scenes_[index];
}

const ast::Shader& AssetAsync::GetShader(u32 index) {
  if (index >= shader_size_) {
    THROW("Fail to get shader");
  }
  return shaders_[index];
}

AssetAsyncLoadTaskSet::AssetAsyncLoadTaskSet(AssetAsync* asset_async)
    : asset_async_{asset_async} {
  m_SetSize = asset_async_->GetSetSize();
}

void AssetAsyncLoadTaskSet::ExecuteRange(enki::TaskSetPartition range,
                                         uint32_t thread_num) {
  for (auto i{range.start}; i < range.end; ++i) {
    if (i < asset_async_->GetSceneSize()) {
      asset_async_->LoadScene(i);
      LOGI("Load scene {} in range {} thread {}", i, i, thread_num);
    } else {
      auto index = i - asset_async_->GetSceneSize();
      asset_async_->LoadShader(index);
      LOGI("Load shader {} in range {} thread {}", index, i, thread_num);
    }
  }
}

Asset::Asset(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu)
    : config_{config},
      gpu_{gpu},
      asset_async_{config_, gpu_},
      asset_async_load_task_set_{&asset_async_} {
  enki::TaskSchedulerConfig task_scheduler_config;
  task_scheduler_config.numTaskThreadsToCreate = 2;
  task_scheduler_.Initialize(task_scheduler_config);
  task_scheduler_.AddTaskSetToPipe(&asset_async_load_task_set_);
}

void Asset::Tick() {}

const ast::Scene& Asset::GetScene(u32 index) {
  WaitAssetAsyncLoad();
  return asset_async_.GetScene(index);
}

const ast::Shader& Asset::GetShader(u32 index) {
  WaitAssetAsyncLoad();
  return asset_async_.GetShader(index);
}

void Asset::WaitAssetAsyncLoad() {
  if (dirty_) {
    task_scheduler_.WaitforTask(&asset_async_load_task_set_);
    dirty_ = false;
  }
}

}  // namespace luka
