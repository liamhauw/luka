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

  u32 GetShaderSize() const;
  u32 GetSetSize() const;
  u32 GetSceneSize() const;

  void LoadScene(u32 index);
  void LoadShader(u32 index);

  const ast::Scene& GetScene(u32 index);
  const ast::Shader& GetShader(u32 index);

 private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<Gpu> gpu_;

  const std::vector<cfg::Scene>* cfg_scenes_;
  const std::vector<cfg::Shader>* cfg_shaders_;
  u32 scene_size_;
  u32 shader_size_;
  u32 set_size_;

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
  void WaitAssetAsyncLoad();

  std::shared_ptr<Config> config_;
  std::shared_ptr<Gpu> gpu_;

  AssetAsync asset_async_;
  AssetAsyncLoadTaskSet asset_async_load_task_set_;
  enki::TaskScheduler task_scheduler_;

  bool dirty_{true};
};

}  // namespace luka