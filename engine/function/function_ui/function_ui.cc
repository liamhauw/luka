/*
  SPDX license identifier: MIT.
  Copyright (C) 2023 Liam Hauw.
*/

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/function_ui/function_ui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "context.h"
#include "imgui.h"

namespace luka {

FunctionUi::FunctionUi() : gpu_{gContext.gpu} {
  CreateImgui();
  AddViewportImage();
}

FunctionUi::~FunctionUi() {
  gpu_->WaitIdle();
  DestroyImgui();
}

void FunctionUi::Tick() {
  if (gContext.window->GetIconified()) {
    return;
  }

  if (gContext.window->GetFramebufferResized()) {
    Resize();
    return;
  }

  CreateUi();
  
  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginFrame()};

  gContext.rendering->Render(command_buffer);

  Render(command_buffer);

  gpu_->EndFrame(command_buffer);
}

void FunctionUi::Render(const vk::raii::CommandBuffer& command_buffer) {
  ImGui::Render();
  gpu_->BeginRenderPass(command_buffer);
  ImDrawData* draw_data{ImGui::GetDrawData()};
  ImGui_ImplVulkan_RenderDrawData(
      draw_data, static_cast<VkCommandBuffer>(*command_buffer));
  gpu_->EndRenderPass(command_buffer);
}

void FunctionUi::Resize() {
  gpu_->WaitIdle();
  ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(descriptor_set_));
  AddViewportImage();
}

void FunctionUi::CreateImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplGlfw_InitForVulkan(gContext.window->GetGlfwWindow(), true);

  auto vulkan_info{gpu_->GetVulkanInfoForImgui()};
  ImGui_ImplVulkan_InitInfo init_info{vulkan_info.first};
  ImGui_ImplVulkan_Init(&init_info, vulkan_info.second);
}

void FunctionUi::AddViewportImage() {
  auto image{gContext.rendering->GetViewportImage()};
  const vk::raii::Sampler& sampler = image.first;
  const vk::raii::ImageView& image_view = image.second;

  descriptor_set_ = ImGui_ImplVulkan_AddTexture(
      *sampler, *image_view,
      static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));
}

void FunctionUi::DestroyImgui() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void FunctionUi::CreateUi() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
  ImGui::Begin("viewport");
  ImGui::Image(descriptor_set_, {1270, 860});
  ImGui::End();
  ImGui::PopStyleVar();
}

}  // namespace luka
