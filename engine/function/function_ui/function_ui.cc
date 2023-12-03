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

FunctionUI::FunctionUI() : gpu_{gContext.gpu} {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplGlfw_InitForVulkan(gContext.window->GetGlfwWindow(), true);
  ImGui_ImplVulkan_InitInfo init_info{gpu_->GetVulkanInitInfoForImgui()};
  ImGui_ImplVulkan_Init(&init_info, gpu_->GetRenderPassForImGui());
}

FunctionUI::~FunctionUI() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void FunctionUI::Tick() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("viewport");
  ImGui::Image(descriptor_set_, {640, 380});
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::Render();
}

void FunctionUI::SetViewportImage(vk::DescriptorSet descriptor_set) {
  descriptor_set_ = descriptor_set;
}

void FunctionUI::Render(const vk::raii::CommandBuffer& command_buffer) {
  gpu_->BeginRenderPass(command_buffer);
  ImDrawData* draw_data{ImGui::GetDrawData()};
  ImGui_ImplVulkan_RenderDrawData(
      draw_data, static_cast<VkCommandBuffer>(*command_buffer));
  gpu_->EndRenderPass(command_buffer);
}

}  // namespace luka
