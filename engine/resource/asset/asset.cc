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
      cfg_scene_paths_{&(config_->GetScenePaths())},
      cfg_shader_paths_{&(config_->GetShaderPaths())},
      cfg_frame_graph_paths_{&(config_->GetFrameGraphPaths())},
      scene_count_{static_cast<u32>(cfg_scene_paths_->size())},
      shader_count_{static_cast<u32>(cfg_shader_paths_->size())},
      frame_graph_count_{static_cast<u32>(cfg_frame_graph_paths_->size())},
      asset_count_{scene_count_ + shader_count_ + frame_graph_count_},
      scenes_(scene_count_),
      shaders_(shader_count_),
      frame_graphs_(frame_graph_count_) {}

u32 AssetAsync::GetSceneCount() const { return scene_count_; }

u32 AssetAsync::GetShaderCount() const { return shader_count_; }

u32 AssetAsync::GetFrameGraphCount() const { return frame_graph_count_; }

u32 AssetAsync::GetAssetCount() const { return asset_count_; }

void AssetAsync::LoadScene(u32 index) {
  scenes_[index] = std::move(ast::Scene{gpu_, (*cfg_scene_paths_)[index]});
}

void AssetAsync::LoadShader(u32 index) {
  shaders_[index] = std::move(ast::Shader{(*cfg_shader_paths_)[index]});
}

void AssetAsync::LoadFrameGraph(u32 index) {
  frame_graphs_[index] =
      std::move(ast::FrameGraph{(*cfg_frame_graph_paths_)[index]});
}

const ast::Scene& AssetAsync::GetScene(u32 index) {
  if (index >= scene_count_) {
    THROW("Fail to get shader");
  }
  return scenes_[index];
}

const ast::Shader& AssetAsync::GetShader(u32 index) {
  if (index >= shader_count_) {
    THROW("Fail to get shader");
  }
  return shaders_[index];
}

const ast::FrameGraph& AssetAsync::GetFrameGraph(u32 index) {
  if (index >= frame_graph_count_) {
    THROW("Fail to get frame graph");
  }
  return frame_graphs_[index];
}

AssetAsyncLoadTaskSet::AssetAsyncLoadTaskSet(AssetAsync* asset_async)
    : asset_async_{asset_async} {
  m_SetSize = asset_async_->GetAssetCount();
}

void AssetAsyncLoadTaskSet::ExecuteRange(enki::TaskSetPartition range,
                                         uint32_t thread_num) {
  u32 scene_count{asset_async_->GetSceneCount()};
  u32 shader_count{asset_async_->GetShaderCount()};
  u32 frame_graph_count{asset_async_->GetFrameGraphCount()};

  for (auto i{range.start}; i < range.end; ++i) {
    if (i < scene_count) {
      asset_async_->LoadScene(i);
      LOGI("Load scene {} in range {} thread {}", i, i, thread_num);
    } else if (i < scene_count + shader_count) {
      u32 index{i - scene_count};
      asset_async_->LoadShader(index);
      LOGI("Load shader {} in range {} thread {}", index, i, thread_num);
    } else {
      u32 index{i - scene_count - shader_count};
      asset_async_->LoadFrameGraph(index);
      LOGI("Load frame graph {} in range {} thread {}", index, i, thread_num);
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

const ast::FrameGraph& Asset::GetFrameGraph(u32 index) {
  WaitAssetAsyncLoad();
  return asset_async_.GetFrameGraph(index);
}

void Asset::WaitAssetAsyncLoad() {
  if (dirty_) {
    task_scheduler_.WaitforTask(&asset_async_load_task_set_);
    dirty_ = false;
  }
}

}  // namespace luka
