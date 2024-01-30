// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/gpu.h"
#include "function/gpu/image.h"

namespace luka {

namespace rd {

class Frame {
 public:
  Frame(std::shared_ptr<Gpu> gpu);

  u32 AddImages(std::vector<gpu::Image>&& images);    
  u32 AddImageViews(std::vector<vk::raii::ImageView>&& image_views);    
  u32 AddFramebuffer(vk::raii::Framebuffer&& framebuffer);    

  const vk::raii::Semaphore& GetRenderFinishedSemphore() const;
  const vk::raii::Fence& GetCommandFinishedFence() const;

  const vk::raii::CommandBuffer& GetActiveCommandBuffer();

  const vk::raii::Framebuffer& GetFramebuffer(u32 i) const;

 private:
  void CreateSyncObjects();
  void CreateCommandObjects();
  void CreateDescriptorObjects();

  std::shared_ptr<Gpu> gpu_;

  // [pass_index][attchment_index]
  std::vector<std::vector<gpu::Image>> images_;
  std::vector<std::vector<vk::raii::ImageView>> image_views_;
  // [pass_index]
  std::vector<vk::raii::Framebuffer> framebuffers_;   

  vk::raii::Semaphore render_finished_semaphore_{nullptr};
  vk::raii::Fence command_finished_fence_{nullptr};

  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::CommandBuffers command_buffers_{nullptr};

  vk::raii::DescriptorPool normal_descriptor_pool_{nullptr};
  vk::raii::DescriptorSets normal_descriptor_sets_{nullptr};
  vk::raii::DescriptorPool bindless_descriptor_pool_{nullptr};
  vk::raii::DescriptorSet bindless_descriptor_set_{nullptr};

};

}  // namespace rd

}  // namespace luka
