// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/framework/framework.h"

#include "core/log.h"

namespace luka {

void RecordGraphicsCommand(
    const vk::raii::CommandBuffer& command_buffer, const fw::Subpass& subpass,
    const fw::DrawElement& draw_element,
    const vk::raii::Pipeline*& prev_pipeline,
    const vk::raii::PipelineLayout*& prev_pipeline_layout, u32 frame_index) {
  // Bind pipeline.
  const vk::raii::Pipeline* pipeline{draw_element.pipeline};
  if (prev_pipeline != pipeline) {
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
    prev_pipeline = pipeline;
  }

  // Push constants, subpass and bindless descriptor sets.
  const vk::raii::PipelineLayout* pipeline_layout{draw_element.pipeline_layout};
  if (prev_pipeline_layout != pipeline_layout) {
    if (subpass.HasPushConstant()) {
      subpass.PushConstants(command_buffer, **pipeline_layout);
    }

    if (subpass.HasSubpassDescriptorSet()) {
      u32 subpass_descriptor_set_index{subpass.GetSubpassDescriptorSetIndex()};
      const vk::raii::DescriptorSet& subpass_descriptor_set{
          subpass.GetSubpassDescriptorSet(frame_index)};
      command_buffer.bindDescriptorSets(
          vk::PipelineBindPoint::eGraphics, **pipeline_layout,
          subpass_descriptor_set_index, *subpass_descriptor_set, nullptr);
    }

    if (subpass.HasBindlessDescriptorSet()) {
      u32 bindless_descriptor_set_index{
          subpass.GetBindlessDescriptorSetIndex()};
      const vk::raii::DescriptorSet& bindless_descriptor_set{
          subpass.GetBindlessDescriptorSet()};
      command_buffer.bindDescriptorSets(
          vk::PipelineBindPoint::eGraphics, **pipeline_layout,
          bindless_descriptor_set_index, *bindless_descriptor_set, nullptr);
    }

    prev_pipeline_layout = pipeline_layout;
  }

  // Bind draw element descriptor sets.
  if (draw_element.has_descriptor_set) {
    u32 draw_element_descriptor_set_index{
        subpass.GetDrawElementDescriptorSetIndex()};

    const vk::raii::DescriptorSets& descriptor_sets{
        draw_element.descriptor_sets[frame_index]};

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

CommandRecord::CommandRecord(
    std::shared_ptr<Config> config,
    const std::vector<vk::raii::CommandBuffers>& secondary_buffers,
    const fw::Subpass& subpass,
    const std::vector<fw::DrawElement>& draw_elements,
    const vk::Viewport& viewport, const vk::Rect2D& scissor, u32 frame_index,
    u32 thread_count, u32 scm_index)
    : config_{std::move(config)},
      secondary_buffers_{&secondary_buffers},
      subpass_{&subpass},
      draw_elements_{&draw_elements},
      viewport_{&viewport},
      scissor_{&scissor},
      frame_index_{frame_index},
      thread_count_{thread_count},
      scm_index_{scm_index},
      draw_element_count_{static_cast<u32>((*draw_elements_).size())},
      prev_pipeline_(thread_count_),
      prev_pipeline_layout_(thread_count_) {}

void CommandRecord::Record(enki::TaskSetPartition range, u32 thread_num) {
  const vk::raii::CommandBuffer& command_buffer{
      (*secondary_buffers_)[thread_num][scm_index_]};

  for (u32 i{range.start}; i < range.end; ++i) {
    const fw::DrawElement& draw_element{(*draw_elements_)[i]};

    if (draw_element.has_scene &&
        !(config_->GetGlobalContext().show_scenes[draw_element.scene_index])) {
      continue;
    }

    command_buffer.setViewport(0, *viewport_);
    command_buffer.setScissor(0, *scissor_);

    RecordGraphicsCommand(command_buffer, *subpass_, draw_element,
                          prev_pipeline_[thread_num],
                          prev_pipeline_layout_[thread_num], frame_index_);
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
                     std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu,
                     std::shared_ptr<Config> config,
                     std::shared_ptr<Asset> asset,
                     std::shared_ptr<Camera> camera,
                     std::shared_ptr<FunctionUi> function_ui)
    : task_scheduler_{std::move(task_scheduler)},
      window_{std::move(window)},
      gpu_{std::move(gpu)},
      config_{std::move(config)},
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
  vk::SemaphoreTypeCreateInfo timeline_semaphore_type_ci{
      vk::SemaphoreType::eTimeline};
  vk::SemaphoreCreateInfo timeline_semaphore_ci{{},
                                                &timeline_semaphore_type_ci};

  graphics_timeline_semaphore_ =
      gpu_->CreateSemaphoreLuka(timeline_semaphore_ci, "timeline");

  vk::SemaphoreCreateInfo semaphore_ci;
  for (u32 i{}; i < frame_count_; ++i) {
    image_acquired_semaphores_.push_back(
        gpu_->CreateSemaphoreLuka(semaphore_ci, "image_acquired"));
    graphics_finished_semaphores_.push_back(
        gpu_->CreateSemaphoreLuka(semaphore_ci, "graphics_finished"));
  }

  // https://github.com/ocornut/imgui/issues/7236
  image_acquired_semaphores_.push_back(
      gpu_->CreateSemaphoreLuka(semaphore_ci, "image_acquired"));
}

void Framework::CreateCommandObjects() {
  vk::CommandPoolCreateInfo primary_command_pool_ci{
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      gpu_->GetGraphicsQueueIndex()};
  vk::CommandBufferAllocateInfo primary_command_buffer_ai{
      nullptr, vk::CommandBufferLevel::ePrimary, 1};

  vk::CommandPoolCreateInfo secondary_command_pool_ci{
      {}, gpu_->GetGraphicsQueueIndex()};
  vk::CommandBufferAllocateInfo secondary_command_buffer_ai{
      nullptr, vk::CommandBufferLevel::eSecondary,
      kMaxSecondaryCommandBufferCount};

  secondary_command_pools_.resize(frame_count_);
  secondary_command_buffers_.resize(frame_count_);
  for (u32 i{}; i < frame_count_; ++i) {
    primary_command_pools_.push_back(
        gpu_->CreateCommandPool(primary_command_pool_ci, "primary"));
    primary_command_buffer_ai.commandPool = *(primary_command_pools_.back());
    primary_command_buffers_.push_back(
        gpu_->AllocateCommandBuffers(primary_command_buffer_ai, "primary"));

    for (u32 j{}; j < thread_count_; ++j) {
      secondary_command_pools_[i].push_back(
          gpu_->CreateCommandPool(secondary_command_pool_ci, "secondary"));
      secondary_command_buffer_ai.commandPool =
          *(secondary_command_pools_[i].back());
      secondary_command_buffers_[i].push_back(gpu_->AllocateCommandBuffers(
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

  for (const auto& enabled_scene : enabled_scenes) {
    config_->GetGlobalContext().show_scenes.emplace(enabled_scene.index, true);
  }

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
  Begin();
  UpdatePasses();
  DrawPasses();
  End();
}

void Framework::Begin() {
  if (absolute_frame_ >= frame_count_) {
    u64 graphics_timeline_value{absolute_frame_ - (frame_count_ - 1)};

    vk::SemaphoreWaitInfo semaphore_wi{
        {}, *graphics_timeline_semaphore_, graphics_timeline_value};

    gpu_->WaitSemaphores(semaphore_wi);
  }

  vk::Result result{};
  std::tie(result, frame_index_) = swapchain_->acquireNextImage(
      UINT64_MAX,
      *(image_acquired_semaphores_[image_acquired_semaphore_index_]), nullptr);

  const vk::raii::CommandBuffer& primary_command_buffer{
      primary_command_buffers_[frame_index_][0]};
  primary_command_buffer.reset({});

  for (u32 j{}; j < thread_count_; ++j) {
    secondary_command_pools_[frame_index_][j].reset();
  }
}

void Framework::UpdatePasses() {
  for (auto& pass : passes_) {
    std::vector<fw::Subpass>& subpasses{pass.GetSubpasses()};
    for (auto& subpass : subpasses) {
      subpass.Update(frame_index_);
    }
  }
}

void Framework::DrawPasses() {
  const vk::raii::CommandBuffer& primary_command_buffer{
      primary_command_buffers_[frame_index_][0]};

  primary_command_buffer.begin({});

  u32 scm_index{};
  // Tarverse passes.
  for (const auto& pass : passes_) {
#ifndef NDEBUG
    gpu_->BeginLabel(primary_command_buffer, "Pass " + pass.GetName(),
                     {0.549F, 0.478F, 0.663F, 1.0F});
#endif
    const vk::RenderPassBeginInfo& render_pass_bi{
        pass.GetRenderPassBeginInfo(frame_index_)};

    // Tarverse subpasses.
    const std::vector<fw::Subpass>& subpasses{pass.GetSubpasses()};
    for (u32 i{}; i < subpasses.size(); ++i) {
      const fw::Subpass& subpass{subpasses[i]};
#ifndef NDEBUG
      gpu_->BeginLabel(primary_command_buffer, "Subpass " + subpass.GetName(),
                       {0.443F, 0.573F, 0.745F, 1.0F});
#endif

      const std::vector<fw::DrawElement>& draw_elements{
          subpass.GetDrawElements()};

      bool use_secondary_command_buffer{draw_elements.size() > 10};
      vk::SubpassContents subpass_contents{
          use_secondary_command_buffer
              ? vk::SubpassContents::eSecondaryCommandBuffers
              : vk::SubpassContents::eInline};

      if (i == 0) {
        primary_command_buffer.beginRenderPass(render_pass_bi,
                                               subpass_contents);
      } else {
        primary_command_buffer.nextSubpass(subpass_contents);
      }

      if (use_secondary_command_buffer) {
        vk::CommandBufferInheritanceInfo inheritance_info{
            render_pass_bi.renderPass, i, render_pass_bi.framebuffer};

        for (u32 j{}; j < thread_count_; ++j) {
          vk::CommandBufferBeginInfo command_buffer_bi{
              vk::CommandBufferUsageFlagBits::eRenderPassContinue,
              &inheritance_info};
          secondary_command_buffers_[frame_index_][j][scm_index].begin(
              command_buffer_bi);
        }

        CommandRecord command_record{
            config_,      secondary_command_buffers_[frame_index_],
            subpass,      draw_elements,
            viewport_,    scissor_,
            frame_index_, thread_count_,
            scm_index};
        CommandRecordTaskSet command_record_task_set{&command_record};
        task_scheduler_->AddTaskSetToPipe(&command_record_task_set);

        task_scheduler_->WaitforTask(&command_record_task_set);

        for (u32 j{}; j < thread_count_; ++j) {
          secondary_command_buffers_[frame_index_][j][scm_index].end();
        }

        std::vector<vk::CommandBuffer> command_buffers;
        for (u32 j{}; j < thread_count_; ++j) {
          command_buffers.push_back(
              *(secondary_command_buffers_[frame_index_][j][scm_index]));
        }

        primary_command_buffer.executeCommands(command_buffers);

        ++scm_index;
      } else {
        primary_command_buffer.setViewport(0, viewport_);
        primary_command_buffer.setScissor(0, scissor_);

        const vk::raii::Pipeline* prev_pipeline{};
        const vk::raii::PipelineLayout* prev_pipeline_layout{};
        for (const fw::DrawElement& draw_element : draw_elements) {
          if (draw_element.has_scene &&
              !(config_->GetGlobalContext()
                    .show_scenes[draw_element.scene_index])) {
            continue;
          }
          RecordGraphicsCommand(primary_command_buffer, subpass, draw_element,
                                prev_pipeline, prev_pipeline_layout,
                                frame_index_);
        }
      }

      // Ui.
      if (pass.HasUi()) {
        function_ui_->Render(primary_command_buffer);
      }

#ifndef NDEBUG
      gpu_->EndLabel(primary_command_buffer);
#endif
    }

    primary_command_buffer.endRenderPass();
#ifndef NDEBUG
    gpu_->EndLabel(primary_command_buffer);
#endif
  }

  primary_command_buffer.end();
}

void Framework::End() {
  // Submit.
  const vk::raii::Queue& graphics_queue{gpu_->GetGraphicsQueue()};

  std::vector<vk::SemaphoreSubmitInfo> wait_semaphore_sis{
      {*(image_acquired_semaphores_[image_acquired_semaphore_index_]), 0,
       vk::PipelineStageFlagBits2::eColorAttachmentOutput}};

  vk::CommandBufferSubmitInfo command_buffer_si{
      *(primary_command_buffers_[frame_index_][0])};

  std::vector<vk::SemaphoreSubmitInfo> signal_semaphore_sis{
      {*(graphics_finished_semaphores_[frame_index_]), 0,
       vk::PipelineStageFlagBits2::eColorAttachmentOutput},
      {*graphics_timeline_semaphore_, absolute_frame_ + 1,
       vk::PipelineStageFlagBits2::eColorAttachmentOutput}};

  vk::SubmitInfo2 si2{
      {}, wait_semaphore_sis, command_buffer_si, signal_semaphore_sis};

  graphics_queue.submit2(si2);

  // Present.
  const vk::raii::Queue& present_queue{gpu_->GetPresentQueue()};

  vk::PresentInfoKHR present_info{
      *(graphics_finished_semaphores_[frame_index_]), **swapchain_,
      frame_index_};

  vk::Result result{present_queue.presentKHR(present_info)};
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    THROW("Fail to present.");
  }

  ++absolute_frame_;
  image_acquired_semaphore_index_ =
      (image_acquired_semaphore_index_ + 1) % (frame_count_ + 1);
}

}  // namespace luka
