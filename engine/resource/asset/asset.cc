// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/asset.h"

#include "core/log.h"

namespace luka {

AssetAsync::AssetAsync(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu,
                       u32 thread_count)
    : config_{config},
      gpu_{gpu},
      thread_count_{thread_count},
      cfg_scene_paths_{&(config_->GetScenePaths())},
      cfg_shader_paths_{&(config_->GetShaderPaths())},
      cfg_frame_graph_paths_{&(config_->GetFrameGraphPaths())},
      scene_count_{static_cast<u32>(cfg_scene_paths_->size())},
      shader_count_{static_cast<u32>(cfg_shader_paths_->size())},
      frame_graph_count_{static_cast<u32>(cfg_frame_graph_paths_->size())},
      asset_count_{scene_count_ + shader_count_ + frame_graph_count_},
      scenes_(scene_count_),
      shaders_(shader_count_),
      frame_graphs_(frame_graph_count_),
      staging_buffers_(thread_count_) {
  vk::CommandPoolCreateInfo command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      gpu_->GetTransferQueueIndex()};
  vk::CommandBufferAllocateInfo command_buffer_allocate_info{
      nullptr, vk::CommandBufferLevel::ePrimary, 1};
  vk::CommandBufferBeginInfo command_buffer_begin_info{
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

  for (u32 i{0}; i < thread_count_; ++i) {
    transfer_command_pools_.push_back(
        std::move(gpu_->CreateCommandPool(command_pool_ci)));
    command_buffer_allocate_info.commandPool = *(transfer_command_pools_[i]);
    transfer_command_buffers_.push_back(std::move(
        gpu_->AllocateCommandBuffers(command_buffer_allocate_info).front()));
    transfer_command_buffers_[i].begin(command_buffer_begin_info);
  }
}

void AssetAsync::Load(enki::TaskSetPartition range, u32 thread_num) {
  for (auto i{range.start}; i < range.end; ++i) {
    if (i < scene_count_) {
      LoadScene(i, thread_num);
      LOGI("Load scene {} in range {} thread {}", i, i, thread_num);
    } else if (i < scene_count_ + shader_count_) {
      u32 index{i - scene_count_};
      LoadShader(index);
      LOGI("Load shader {} in range {} thread {}", index, i, thread_num);
    } else {
      u32 index{i - scene_count_ - shader_count_};
      LoadFrameGraph(index);
      LOGI("Load frame graph {} in range {} thread {}", index, i, thread_num);
    }
  }
}

void AssetAsync::Submit() {
  std::vector<vk::CommandBuffer> command_buffers;

  for (u32 i{0}; i < thread_count_; ++i) {
    transfer_command_buffers_[i].end();
    command_buffers.push_back(*transfer_command_buffers_[i]);
  }
  vk::SubmitInfo submit_info{nullptr, nullptr, command_buffers};
  gpu_->TransferQueueSubmit(submit_info);
}

u32 AssetAsync::GetAssetCount() const { return asset_count_; }

void AssetAsync::LoadScene(u32 index, u32 thread_num) {
  scenes_[index] = std::move(ast::Scene{gpu_, (*cfg_scene_paths_)[index],
                                        transfer_command_buffers_[thread_num],
                                        staging_buffers_[thread_num]});
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
  asset_async_->Load(range, thread_num);
}

Asset::Asset(std::shared_ptr<Config> config,
             std::shared_ptr<TaskScheduler> task_scheduler,
             std::shared_ptr<Gpu> gpu)
    : config_{config},
      task_scheduler_{task_scheduler},
      gpu_{gpu},
      asset_async_{config_, gpu_, task_scheduler_->GetThreadCount()},
      asset_async_load_task_set_{&asset_async_} {
  task_scheduler_->AddTaskSetToPipe(&asset_async_load_task_set_);
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
  if (!loaded_) {
    task_scheduler_->WaitforTask(&asset_async_load_task_set_);
    asset_async_.Submit();
    loaded_ = true;
  }
}

}  // namespace luka
