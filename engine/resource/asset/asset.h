// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <TaskScheduler.h>

#include "resource/asset/frame_graph.h"
#include "resource/asset/light.h"
#include "resource/asset/scene.h"
#include "resource/asset/shader.h"
#include "resource/config/config.h"
#include "resource/gpu/gpu.h"
#include "resource/task_scheduler/task_scheduler.h"

namespace luka {

class AssetAsync {
 public:
  AssetAsync(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu,
             u32 thread_count);

  void Load(enki::TaskSetPartition range, u32 thread_num);

  void Submit();

  u32 GetAssetCount() const;

  const ast::Scene& GetScene(u32 index);
  const ast::Light& GetLight(u32 index);
  const ast::Shader& GetShader(u32 index);
  const ast::FrameGraph& GetFrameGraph(u32 index);

 private:
  void LoadScene(u32 index, u32 thread_num);
  void LoadLight(u32 index);
  void LoadShader(u32 index);
  void LoadFrameGraph(u32 index);

  std::shared_ptr<Config> config_;
  std::shared_ptr<Gpu> gpu_;
  u32 thread_count_;

  const std::vector<std::filesystem::path>* cfg_scene_paths_;
  const std::vector<std::filesystem::path>* cfg_light_paths_;
  const std::vector<std::filesystem::path>* cfg_shader_paths_;
  const std::vector<std::filesystem::path>* cfg_frame_graph_paths_;

  u32 scene_count_;
  u32 light_count_;
  u32 shader_count_;
  u32 frame_graph_count_;
  u32 asset_count_;

  std::vector<ast::Scene> scenes_;
  std::vector<ast::Light> lights_;
  std::vector<ast::Shader> shaders_;
  std::vector<ast::FrameGraph> frame_graphs_;

  std::vector<vk::raii::CommandPool> transfer_command_pools_;
  vk::raii::CommandBuffers transfer_command_buffers_{nullptr};
  std::vector<std::vector<gpu::Buffer>> staging_buffers_;
};

class AssetAsyncLoadTaskSet : public enki::ITaskSet {
 public:
  AssetAsyncLoadTaskSet() = default;

  AssetAsyncLoadTaskSet(AssetAsync* asset_async);

  void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override;

 private:
  AssetAsync* asset_async_{nullptr};
};

class Asset {
 public:
  Asset(std::shared_ptr<Config> config,
        std::shared_ptr<TaskScheduler> task_scheduler,
        std::shared_ptr<Gpu> gpu);

  void Tick();

  const ast::Scene& GetScene(u32 index);
  const ast::Light& GetLight(u32 index);
  const ast::Shader& GetShader(u32 index);
  const ast::FrameGraph& GetFrameGraph(u32 index);

 private:
  void WaitAssetAsyncLoad();

  std::shared_ptr<Config> config_;
  std::shared_ptr<TaskScheduler> task_scheduler_;
  std::shared_ptr<Gpu> gpu_;

  AssetAsync asset_async_;
  AssetAsyncLoadTaskSet asset_async_load_task_set_;
  bool loaded_{false};
};

}  // namespace luka