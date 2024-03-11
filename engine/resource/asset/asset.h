// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <TaskScheduler.h>

#include "core/log.h"
#include "resource/asset/scene.h"
#include "resource/asset/shader.h"
#include "resource/config/config.h"
#include "resource/gpu/gpu.h"
namespace luka {

class AssetAsync {
 public:
  AssetAsync(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu);

  void Load();

  const ast::Scene& GetScene(u32 index);
  const ast::Shader& GetShader(u32 index);

 private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<Gpu> gpu_;

  std::vector<ast::Scene> scenes_;
  std::vector<ast::Shader> shaders_;
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
  Asset(std::shared_ptr<Config> config, std::shared_ptr<Gpu> gpu);

  void Tick();

  const ast::Scene& GetScene(u32 index);
  const ast::Shader& GetShader(u32 index);

 private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<Gpu> gpu_;

  AssetAsync asset_async_;
  AssetAsyncLoadTaskSet asset_async_load_task_set_;
  enki::TaskScheduler task_scheduler_;
};

}  // namespace luka