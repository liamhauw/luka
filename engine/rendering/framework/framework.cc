// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/framework/framework.h"

#include "core/log.h"

namespace luka {

CommandRecord::CommandRecord(
    std::shared_ptr<Config> config, u32 thread_count, u32 frame_index,
    const fw::Subpass& subpass,
    const std::vector<fw::DrawElement>& draw_elements,
    const vk::Viewport& viewport, const vk::Rect2D& scissor,
    const std::vector<std::vector<vk::raii::CommandBuffers>>& secondary_buffers,
    u32 scm_index)
    : config_{std::move(config)},
      thread_count_{thread_count},
      frame_index_{frame_index},
      subpass_{&subpass},
      draw_elements_{&draw_elements},
      draw_element_count_{static_cast<u32>((*draw_elements_).size())},
      viewport_{&viewport},
      scissor_{&scissor},
      secondary_buffers_{&secondary_buffers},
      scm_index_{scm_index},
      prev_pipeline_(thread_count_),
      prev_pipeline_layout_(thread_count_) {}

void CommandRecord::Record(enki::TaskSetPartition range, u32 thread_num) {
  const vk::raii::CommandBuffer& command_buffer{
      (*secondary_buffers_)[frame_index_][thread_num][scm_index_]};

  for (u32 i{range.start}; i < range.end; ++i) {
    // LOGI("Record command buffer in range {} thread {}", i, thread_num);
    const fw::DrawElement& draw_element{(*draw_elements_)[i]};

    // Show scene?
    if (draw_element.has_scene &&
        !(config_->GetGlobalContext().show_scenes[draw_element.scene_index])) {
      continue;
    }

    command_buffer.setViewport(0, *viewport_);
    command_buffer.setScissor(0, *scissor_);

    // Bind pipeline.
    const vk::raii::Pipeline* pipeline{draw_element.pipeline};
    if (prev_pipeline_[thread_num] != pipeline) {
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
      prev_pipeline_[thread_num] = pipeline;
    }

    // Push constants, subpass and bindless descriptor sets.
    const vk::raii::PipelineLayout* pipeline_layout{
        draw_element.pipeline_layout};
    if (prev_pipeline_layout_[thread_num] != pipeline_layout) {
      if (subpass_->HasPushConstant()) {
        subpass_->PushConstants(command_buffer, **pipeline_layout);
      }

      if (subpass_->HasSubpassDescriptorSet()) {
        u32 subpass_descriptor_set_index{
            subpass_->GetSubpassDescriptorSetIndex()};
        const vk::raii::DescriptorSet& subpass_descriptor_set{
            subpass_->GetSubpassDescriptorSet(frame_index_)};
        command_buffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, **pipeline_layout,
            subpass_descriptor_set_index, *subpass_descriptor_set, nullptr);
      }

      if (subpass_->HasBindlessDescriptorSet()) {
        u32 bindless_descriptor_set_index{
            subpass_->GetBindlessDescriptorSetIndex()};
        const vk::raii::DescriptorSet& bindless_descriptor_set{
            subpass_->GetBindlessDescriptorSet()};
        command_buffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, **pipeline_layout,
            bindless_descriptor_set_index, *bindless_descriptor_set, nullptr);
      }

      prev_pipeline_layout_[thread_num] = pipeline_layout;
    }

    // Bind draw element descriptor sets.
    if (draw_element.has_descriptor_set) {
      u32 draw_element_descriptor_set_index{
          subpass_->GetDrawElementDescriptorSetIndex()};

      const vk::raii::DescriptorSets& descriptor_sets{
          draw_element.descriptor_sets[frame_index_]};

      std::vector<vk::DescriptorSet> vk_descriptor_sets;
      for (const auto& descriptor_set : descriptor_sets) {
        vk_descriptor_sets.push_back(*(descriptor_set));
      }
      command_buffer.bindDescriptorSets(
          vk::PipelineBindPoint::eGraphics, **pipeline_layout,
          draw_element_descriptor_set_index, vk_descriptor_sets, nullptr);
    }

    // Draw.
    if (draw_element.has_scene) {
      const std::vector<fw::DrawElmentVertexInfo>& vertex_infos{
          draw_element.vertex_infos};

      for (const auto& vertex_info : vertex_infos) {
        command_buffer.bindVertexBuffers(
            vertex_info.location, vertex_info.buffers, vertex_info.offsets);
      }

      if (!draw_element.has_index) {
        command_buffer.draw(draw_element.vertex_count, 1, 0, 0);
      } else {
        const ast::sc::IndexAttribute* index_attribute{
            draw_element.index_attribute};

        command_buffer.bindIndexBuffer(*(index_attribute->buffer),
                                       index_attribute->offset,
                                       index_attribute->index_type);

        command_buffer.drawIndexed(index_attribute->count, 1, 0, 0, 0);
      }
    } else {
      command_buffer.draw(3, 1, 0, 0);
    }
  }
}

u32 CommandRecord::GetDrawElmentCount() const { return draw_element_count_; }

CommandRecordTaskSet::CommandRecordTaskSet(CommandRecord* command_record)
    : command_record_{command_record} {
  m_SetSize = command_record_->GetDrawElmentCount();
}

void CommandRecordTaskSet::ExecuteRange(enki::TaskSetPartition range,
                                        uint32_t thread_num) {
  command_record_->Record(range, thread_num);
}

Framework::Framework(std::shared_ptr<TaskScheduler> task_scheduler,
                     std::shared_ptr<Config> config,
                     std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu,
                     std::shared_ptr<Asset> asset,
                     std::shared_ptr<Camera> camera,
                     std::shared_ptr<FunctionUi> function_ui)
    : task_scheduler_{std::move(task_scheduler)},
      config_{std::move(config)},
      window_{std::move(window)},
      gpu_{std::move(gpu)},
      asset_{std::move(asset)},
      camera_{std::move(camera)},
      function_ui_{std::move(function_ui)},
      thread_count_{task_scheduler_->GetThreadCount()} {
  GetSwapchain();
  CreateSyncObjects();
  CreateCommandObjects();
  CreateViewportAndScissor();
  CreatePasses();
}

Framework::~Framework() { gpu_->WaitIdle(); }

void Framework::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    window_->SetFramebufferResized(false);
    Resize();
  }

  Render();
}

void Framework::GetSwapchain() {
  swapchain_info_ = &(function_ui_->GetSwapchainInfo());
  swapchain_ = &(function_ui_->GetSwapchain());
  swapchain_images_ = swapchain_->getImages();
  frame_count_ = swapchain_images_.size();
}

void Framework::CreateSyncObjects() {
  vk::SemaphoreCreateInfo semaphore_ci;
  vk::FenceCreateInfo fence_ci{vk::FenceCreateFlagBits::eSignaled};
  for (u32 i{}; i < frame_count_; ++i) {
    image_acquired_semaphores_.push_back(
        gpu_->CreateSemaphoreLuka(semaphore_ci, "image_acquired"));
    render_finished_semaphores_.push_back(
        gpu_->CreateSemaphoreLuka(semaphore_ci, "render_finished"));
    command_finished_fences_.push_back(
        gpu_->CreateFence(fence_ci, "command_finished"));
  }

  // https://github.com/ocornut/imgui/issues/7236
  image_acquired_semaphores_.push_back(
      gpu_->CreateSemaphoreLuka(semaphore_ci, "image_acquired"));
}

void Framework::CreateCommandObjects() {
  vk::CommandPoolCreateInfo command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      gpu_->GetGraphicsQueueIndex()};
  vk::CommandBufferAllocateInfo command_buffer_ai{
      nullptr, vk::CommandBufferLevel::ePrimary, 1};

  secondary_pools_.resize(frame_count_);
  secondary_buffers_.resize(frame_count_);
  for (u32 i{}; i < frame_count_; ++i) {
    command_pools_.push_back(
        gpu_->CreateCommandPool(command_pool_ci, "graphics"));
    command_buffer_ai.commandPool = *(command_pools_.back());
    command_buffers_.push_back(
        gpu_->AllocateCommandBuffers(command_buffer_ai, "graphics"));

    for (u32 j{}; j < thread_count_; ++j) {
      vk::CommandPoolCreateInfo secondary_command_pool_ci{
          {}, gpu_->GetGraphicsQueueIndex()};
      vk::CommandBufferAllocateInfo secondary_command_buffer_ai{
          nullptr, vk::CommandBufferLevel::eSecondary, 8};
      secondary_pools_[i].push_back(
          gpu_->CreateCommandPool(secondary_command_pool_ci, "secondary"));
      secondary_command_buffer_ai.commandPool = *(secondary_pools_[i].back());
      secondary_buffers_[i].push_back(gpu_->AllocateCommandBuffers(
          secondary_command_buffer_ai, "secondary"));
    }
  }
}

void Framework::CreateViewportAndScissor() {
  u32 target_width{swapchain_info_->extent.width};
  u32 target_height{swapchain_info_->extent.height};

  viewport_ = vk::Viewport{0.0F,
                           0.0F,
                           static_cast<f32>(target_width),
                           static_cast<f32>(target_height),
                           0.0F,
                           1.0F};
  scissor_ =
      vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{target_width, target_height}};
}

void Framework::CreatePasses() {
  u32 frame_graph_index{config_->GetFrameGraphIndex()};
  const ast::FrameGraph& frame_graph{asset_->GetFrameGraph(frame_graph_index)};

  const std::vector<ast::Pass>& ast_passes{frame_graph.GetPasses()};

  std::vector<fw::ScenePrimitive> scene_primitives;

  const std::vector<ast::EnabledScene>& enabled_scenes{
      frame_graph.GetEnabledScenes()};

  config_->GetGlobalContext().show_scenes =
      std::vector<bool>(enabled_scenes.size(), true);

  for (const auto& enabled_scene : enabled_scenes) {
    const ast::sc::Scene* scene{
        asset_->GetScene(enabled_scene.index).GetScene()};

    const glm::mat4& enabled_scene_model{enabled_scene.model};

    const std::vector<ast::sc::Node*>& nodes{scene->GetNodes()};

    std::queue<const ast::sc::Node*> all_nodes;
    std::unordered_map<const ast::sc::Node*, glm::mat4> node_model_matrix;

    for (const ast::sc::Node* node : nodes) {
      all_nodes.push(node);
    }

    while (!all_nodes.empty()) {
      const ast::sc::Node* cur_node{all_nodes.front()};
      all_nodes.pop();

      const std::vector<ast::sc::Node*>& cur_node_children{
          cur_node->GetChildren()};
      for (const ast::sc::Node* cur_node_child : cur_node_children) {
        all_nodes.push(cur_node_child);
      }

      // Model matrix.
      glm::mat4 model_matrix{enabled_scene_model};

      model_matrix *= cur_node->GetModelMarix();

      const ast::sc::Node* parent_node{cur_node->GetParent()};
      while (parent_node) {
        model_matrix *= parent_node->GetModelMarix();
        parent_node = parent_node->GetParent();
      }

      glm::mat4 inverse_model_matrix{glm::inverse(model_matrix)};

      // Primitives.
      const ast::sc::Mesh* mesh{cur_node->GetMesh()};
      if (!mesh) {
        continue;
      }
      const std::vector<ast::sc::Primitive>& primitives{mesh->GetPrimitives()};

      for (const auto& primitive : primitives) {
        scene_primitives.emplace_back(enabled_scene.index, model_matrix,
                                      inverse_model_matrix, &primitive);
      }
    }
  }

  shared_image_views_.resize(frame_count_);
  for (u32 i{}; i < ast_passes.size(); ++i) {
    passes_.emplace_back(gpu_, asset_, camera_, function_ui_, frame_count_,
                         *swapchain_info_, swapchain_images_, ast_passes, i,
                         scene_primitives, shared_image_views_);
  }
}

void Framework::Resize() {
  GetSwapchain();
  CreateViewportAndScissor();
  for (auto& pass : passes_) {
    pass.Resize(*swapchain_info_, swapchain_images_);
  }
}

void Framework::Render() {
  const vk::raii::CommandBuffer& command_buffer{Begin()};
  UpdatePasses();
  DrawPasses(command_buffer);
  End(command_buffer);
}

const vk::raii::CommandBuffer& Framework::Begin() {
  vk::Result result{};
  std::tie(result, frame_index_) = swapchain_->acquireNextImage(
      UINT64_MAX,
      *(image_acquired_semaphores_[image_acquired_semaphore_index_]), nullptr);

  const vk::raii::Fence& command_finished_fence{
      command_finished_fences_[frame_index_]};
  gpu_->WaitForFence(command_finished_fence);

  gpu_->ResetFence(command_finished_fence);

  const vk::raii::CommandBuffer& command_buffer{
      command_buffers_[frame_index_][0]};
  command_buffer.reset({});

  for (u32 j{}; j < thread_count_; ++j) {
    secondary_pools_[frame_index_][j].reset();
  }

  return command_buffer;
}

void Framework::End(const vk::raii::CommandBuffer& command_buffer) {
  vk::PipelineStageFlags wait_pipeline_stage{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::SubmitInfo submit_info{
      *(image_acquired_semaphores_[image_acquired_semaphore_index_]),
      wait_pipeline_stage, *command_buffer,
      *(render_finished_semaphores_[frame_index_])};

  const vk::raii::Queue& graphics_queue{gpu_->GetGraphicsQueue()};
  graphics_queue.submit(submit_info, *(command_finished_fences_[frame_index_]));

  vk::PresentInfoKHR present_info{*(render_finished_semaphores_[frame_index_]),
                                  **swapchain_, frame_index_};

  const vk::raii::Queue& present_queue{gpu_->GetPresentQueue()};

  vk::Result result{present_queue.presentKHR(present_info)};
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    THROW("Fail to present.");
  }

  image_acquired_semaphore_index_ =
      (image_acquired_semaphore_index_ + 1) % (frame_count_ + 1);
}

void Framework::UpdatePasses() {
  for (auto& pass : passes_) {
    std::vector<fw::Subpass>& subpasses{pass.GetSubpasses()};
    for (auto& subpass : subpasses) {
      subpass.Update(frame_index_);
    }
  }
}

void Framework::DrawPasses(const vk::raii::CommandBuffer& command_buffer) {
  command_buffer.begin({});

  u32 scm_index{};
  // Tarverse passes.
  for (const auto& pass : passes_) {
#ifndef NDEBUG
    gpu_->BeginLabel(command_buffer, "Pass " + pass.GetName(),
                     {0.549F, 0.478F, 0.663F, 1.0F});
#endif
    const vk::RenderPassBeginInfo& render_pass_begin_info{
        pass.GetRenderPassBeginInfo(frame_index_)};

    // Tarverse subpasses.
    const std::vector<fw::Subpass>& subpasses{pass.GetSubpasses()};
    for (u32 i{}; i < subpasses.size(); ++i) {
      const fw::Subpass& subpass{subpasses[i]};
#ifndef NDEBUG
      gpu_->BeginLabel(command_buffer, "Subpass " + subpass.GetName(),
                       {0.443F, 0.573F, 0.745F, 1.0F});
#endif

      const std::vector<fw::DrawElement>& draw_elements{
          subpass.GetDrawElements()};

      bool use_secondary_command_buffer{draw_elements.size() > 10};
      vk::SubpassContents subpass_contents{
          use_secondary_command_buffer
              ? vk::SubpassContents::eSecondaryCommandBuffers
              : vk::SubpassContents::eInline};

      // Next subpass.
      if (i == 0) {
        command_buffer.beginRenderPass(render_pass_begin_info,
                                       subpass_contents);
      } else {
        command_buffer.nextSubpass(subpass_contents);
      }

      if (use_secondary_command_buffer) {
        vk::CommandBufferInheritanceInfo inheritance_info{
            render_pass_begin_info.renderPass, i,
            render_pass_begin_info.framebuffer};

        for (u32 j{}; j < thread_count_; ++j) {
          vk::CommandBufferBeginInfo command_buffer_begin_info{
              vk::CommandBufferUsageFlagBits::eRenderPassContinue,
              &inheritance_info};
          secondary_buffers_[frame_index_][j][scm_index].begin(
              command_buffer_begin_info);
        }

        CommandRecord command_record{config_,  thread_count_,      frame_index_,
                                     subpass,  draw_elements,      viewport_,
                                     scissor_, secondary_buffers_, scm_index};
        CommandRecordTaskSet command_record_task_set{&command_record};
        task_scheduler_->AddTaskSetToPipe(&command_record_task_set);

        task_scheduler_->WaitforTask(&command_record_task_set);

        for (u32 j{}; j < thread_count_; ++j) {
          secondary_buffers_[frame_index_][j][scm_index].end();
        }

        std::vector<vk::CommandBuffer> command_buffers;
        for (u32 j{}; j < thread_count_; ++j) {
          command_buffers.push_back(
              *(secondary_buffers_[frame_index_][j][scm_index]));
        }

        ++scm_index;

        command_buffer.executeCommands(command_buffers);
      } else {
        command_buffer.setViewport(0, viewport_);
        command_buffer.setScissor(0, scissor_);

        const vk::raii::Pipeline* prev_pipeline{};
        const vk::raii::PipelineLayout* prev_pipeline_layout{};
        for (const fw::DrawElement& draw_element : draw_elements) {
          // Show draw_element?
          if (draw_element.has_scene &&
              !(config_->GetGlobalContext()
                    .show_scenes[draw_element.scene_index])) {
            continue;
          }

          // Bind pipeline.
          const vk::raii::Pipeline* pipeline{draw_element.pipeline};
          if (prev_pipeline != pipeline) {
            command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                        **pipeline);
            prev_pipeline = pipeline;
          }

          // Push constants, subpass and bindless descriptor sets.
          const vk::raii::PipelineLayout* pipeline_layout{
              draw_element.pipeline_layout};
          if (prev_pipeline_layout != pipeline_layout) {
            if (subpass.HasPushConstant()) {
              subpass.PushConstants(command_buffer, **pipeline_layout);
            }

            if (subpass.HasSubpassDescriptorSet()) {
              u32 subpass_descriptor_set_index{
                  subpass.GetSubpassDescriptorSetIndex()};
              const vk::raii::DescriptorSet& subpass_descriptor_set{
                  subpass.GetSubpassDescriptorSet(frame_index_)};
              command_buffer.bindDescriptorSets(
                  vk::PipelineBindPoint::eGraphics, **pipeline_layout,
                  subpass_descriptor_set_index, *subpass_descriptor_set,
                  nullptr);
            }

            if (subpass.HasBindlessDescriptorSet()) {
              u32 bindless_descriptor_set_index{
                  subpass.GetBindlessDescriptorSetIndex()};
              const vk::raii::DescriptorSet& bindless_descriptor_set{
                  subpass.GetBindlessDescriptorSet()};
              command_buffer.bindDescriptorSets(
                  vk::PipelineBindPoint::eGraphics, **pipeline_layout,
                  bindless_descriptor_set_index, *bindless_descriptor_set,
                  nullptr);
            }

            prev_pipeline_layout = pipeline_layout;
          }

          // Bind draw element descriptor sets.
          if (draw_element.has_descriptor_set) {
            u32 draw_element_descriptor_set_index{
                subpass.GetDrawElementDescriptorSetIndex()};

            const vk::raii::DescriptorSets& descriptor_sets{
                draw_element.descriptor_sets[frame_index_]};

            std::vector<vk::DescriptorSet> vk_descriptor_sets;
            for (const auto& descriptor_set : descriptor_sets) {
              vk_descriptor_sets.push_back(*(descriptor_set));
            }
            command_buffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, **pipeline_layout,
                draw_element_descriptor_set_index, vk_descriptor_sets, nullptr);
          }

          // Draw.
          if (draw_element.has_scene) {
            const std::vector<fw::DrawElmentVertexInfo>& vertex_infos{
                draw_element.vertex_infos};

            for (const auto& vertex_info : vertex_infos) {
              command_buffer.bindVertexBuffers(vertex_info.location,
                                               vertex_info.buffers,
                                               vertex_info.offsets);
            }

            if (!draw_element.has_index) {
              command_buffer.draw(draw_element.vertex_count, 1, 0, 0);
            } else {
              const ast::sc::IndexAttribute* index_attribute{
                  draw_element.index_attribute};

              command_buffer.bindIndexBuffer(*(index_attribute->buffer),
                                             index_attribute->offset,
                                             index_attribute->index_type);

              command_buffer.drawIndexed(index_attribute->count, 1, 0, 0, 0);
            }
          } else {
            command_buffer.draw(3, 1, 0, 0);
          }
        }
      }

      // Ui.
      if (pass.HasUi()) {
        function_ui_->Render(command_buffer);
      }

#ifndef NDEBUG
      gpu_->EndLabel(command_buffer);
#endif
    }

    // End render pass.
    command_buffer.endRenderPass();
#ifndef NDEBUG
    gpu_->EndLabel(command_buffer);
#endif
  }

  command_buffer.end();
}

}  // namespace luka
