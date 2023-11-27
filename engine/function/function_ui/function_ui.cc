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

FunctionUI::FunctionUI() {
  // IMGUI_CHECKVERSION();
  // ImGui::CreateContext();

  // ImGuiIO& io = ImGui::GetIO();
  // io.IniFilename = nullptr;
  // io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

  // ImGui_ImplGlfw_InitForVulkan(gContext.window->GetGlfwWindow(), true);

  // ImGui_ImplVulkan_InitInfo init_info{
  //     gContext.gpu->GetVulkanInitInfoForImgui()};
  // ImGui_ImplVulkan_Init(&init_info, gContext.gpu->GetRenderPassForImGui());
}

void FunctionUI::Tick() {}

}  // namespace luka
