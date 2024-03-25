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
#include "imgui.h"

namespace luka {

FunctionUi::FunctionUi(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu)
    : window_{window}, gpu_{gpu} {
  CreateImgui();
}

FunctionUi::~FunctionUi() {
  gpu_->WaitIdle();
  DestroyImgui();
}

void FunctionUi::Tick() { CreateUi(); }

void FunctionUi::Render(const vk::raii::CommandBuffer& command_buffer) {
  ImGui::Render();
  ImDrawData* draw_data{ImGui::GetDrawData()};
  ImGui_ImplVulkan_RenderDrawData(
      draw_data, static_cast<VkCommandBuffer>(*command_buffer));
}

void FunctionUi::CreateImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplGlfw_InitForVulkan(window_->GetGlfwWindow(), true);

  auto vulkan_init_info{gpu_->GetImguiVulkanInitInfo()};
  ImGui_ImplVulkan_Init(&vulkan_init_info);
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

  bool show_demo_window{true};
  ImGui::ShowDemoWindow(&show_demo_window);
}

}  // namespace luka
