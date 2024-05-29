// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <TaskScheduler.h>

#include "base/gpu/gpu.h"
#include "base/task_scheduler//task_scheduler.h"
#include "base/window/window.h"
#include "core/util.h"
#include "function/camera/camera.h"
#include "function/function_ui/function_ui.h"
#include "rendering/framework/pass.h"
#include "resource/asset/asset.h"
#include "resource/config/config.h"

namespace luka {

class CommandRecord {
 public:
  CommandRecord() = default;
  CommandRecord(std::shared_ptr<Config> config, u32 thread_count,
                u32 frame_index, const fw::Subpass& subpass,
                const std::vector<fw::DrawElement>& draw_elements,
                const vk::Viewport& viewport, const vk::Rect2D& scissor,
                const std::vector<std::vector<vk::raii::CommandBuffers>>&
                    secondary_buffers,
                u32 scm_index);

  void Record(enki::TaskSetPartition range, u32 thread_num);

  u32 GetDrawElmentCount() const;

 private:
  std::shared_ptr<Config> config_;

  u32 thread_count_{};
  u32 frame_index_{};
  const fw::Subpass* subpass_{};
  const std::vector<fw::DrawElement>* draw_elements_{};
  u32 draw_element_count_{};
  const vk::Viewport* viewport_{};
  const vk::Rect2D* scissor_{};

  const std::vector<std::vector<vk::raii::CommandBuffers>>*
      secondary_buffers_{};
  u32 scm_index_{};

  std::vector<const vk::raii::Pipeline*> prev_pipeline_;
  std::vector<const vk::raii::PipelineLayout*> prev_pipeline_layout_;
};

class CommandRecordTaskSet : public enki::ITaskSet {
 public:
  CommandRecordTaskSet() = default;
  explicit CommandRecordTaskSet(CommandRecord* command_record);

  void ExecuteRange(enki::TaskSetPartition range, uint32_t thread_num) override;

 private:
  CommandRecord* command_record_{};
};

class Framework {
 public:
  DELETE_SPECIAL_MEMBER_FUNCTIONS(Framework)

  Framework(std::shared_ptr<TaskScheduler> task_scheduler,
            std::shared_ptr<Config> config, std::shared_ptr<Window> window,
            std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
            std::shared_ptr<Camera> camera,
            std::shared_ptr<FunctionUi> function_ui);

  ~Framework();

  void Tick();

 private:
  void GetSwapchain();
  void CreateSyncObjects();
  void CreateCommandObjects();
  void CreateViewportAndScissor();
  void CreatePasses();

  void Resize();
  void Render();

  const vk::raii::CommandBuffer& Begin();
  void End(const vk::raii::CommandBuffer& command_buffer);
  void UpdatePasses();
  void DrawPasses(const vk::raii::CommandBuffer& command_buffer);

  std::shared_ptr<TaskScheduler> task_scheduler_;
  std::shared_ptr<Config> config_;
  std::shared_ptr<Window> window_;
  std::shared_ptr<Gpu> gpu_;
  std::shared_ptr<Asset> asset_;
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<FunctionUi> function_ui_;

  u32 thread_count_{};

  const SwapchainInfo* swapchain_info_{};
  const vk::raii::SwapchainKHR* swapchain_{};
  std::vector<vk::Image> swapchain_images_;
  u32 frame_count_{};

  std::vector<vk::raii::Semaphore> image_acquired_semaphores_;
  std::vector<vk::raii::Semaphore> render_finished_semaphores_;
  std::vector<vk::raii::Fence> command_finished_fences_;

  std::vector<vk::raii::CommandPool> command_pools_;
  std::vector<vk::raii::CommandBuffers> command_buffers_;
  std::vector<std::vector<vk::raii::CommandPool>> secondary_pools_;
  std::vector<std::vector<vk::raii::CommandBuffers>> secondary_buffers_;

  vk::Viewport viewport_;
  vk::Rect2D scissor_;

  std::vector<std::unordered_map<std::string, vk::ImageView>>
      shared_image_views_;
  std::vector<fw::Pass> passes_;

  u32 image_acquired_semaphore_index_{};
  u32 frame_index_{};
};

}  // namespace luka
