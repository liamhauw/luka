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

FunctionUi::FunctionUi() {
  // CreateImgui();
  // AddViewportImage();
}

FunctionUi::~FunctionUi() {
  // gpu_->WaitIdle();
  // DestroyImgui();
}

void FunctionUi::Tick() {
  //   if (gContext.window->GetIconified()) {
  //     return;
  //   }

  //   if (gContext.window->GetFramebufferResized()) {
  //     Resize();
  //     return;
  //   }

  //   CreateUi();

  //   const vk::raii::CommandBuffer& command_buffer{gpu_->BeginFrame()};

  // #ifndef NDEBUG
  //   gpu_->BeginLabel(command_buffer, "rendering");
  // #endif
  //   gContext.rendering->Render(command_buffer);
  // #ifndef NDEBUG
  //   gpu_->EndLabel(command_buffer);
  // #endif

  // #ifndef NDEBUG
  //   gpu_->BeginLabel(command_buffer, "ui");
  // #endif
  //   Render(command_buffer);
  // #ifndef NDEBUG
  //   gpu_->EndLabel(command_buffer);
  // #endif

  //   gpu_->EndFrame(command_buffer);
}

void FunctionUi::Render(const vk::raii::CommandBuffer& command_buffer) {
  // ImGui::Render();
  // gpu_->BeginRenderPass(command_buffer);
  // ImDrawData* draw_data{ImGui::GetDrawData()};
  // ImGui_ImplVulkan_RenderDrawData(
  //     draw_data, static_cast<VkCommandBuffer>(*command_buffer));
  // gpu_->EndRenderPass(command_buffer);
}

void FunctionUi::Resize() {
  // gpu_->WaitIdle();
  // ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(descriptor_set_));
  // AddViewportImage();
}

void FunctionUi::CreateImgui() {
  // IMGUI_CHECKVERSION();
  // ImGui::CreateContext();

  // ImGuiIO& io = ImGui::GetIO();
  // io.IniFilename = nullptr;
  // io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

  // ImGui_ImplGlfw_InitForVulkan(gContext.window->GetGlfwWindow(), true);

  // auto vulkan_info{gpu_->GetVulkanInfoForImgui()};
  // ImGui_ImplVulkan_InitInfo init_info{vulkan_info.first};
  // ImGui_ImplVulkan_Init(&init_info, vulkan_info.second);
}

void FunctionUi::AddViewportImage() {
  // auto image{gContext.rendering->GetViewportImage()};
  // const vk::raii::Sampler& sampler = image.first;
  // const vk::raii::ImageView& image_view = image.second;

  // descriptor_set_ = ImGui_ImplVulkan_AddTexture(
  //     *sampler, *image_view,
  //     static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));
}

void FunctionUi::DestroyImgui() {
  // ImGui_ImplVulkan_Shutdown();
  // ImGui_ImplGlfw_Shutdown();
  // ImGui::DestroyContext();
}

void FunctionUi::CreateUi() {
  // ImGui_ImplVulkan_NewFrame();
  // ImGui_ImplGlfw_NewFrame();
  // ImGui::NewFrame();

  // vk::Extent2D extent2d{gpu_->GetExtent2D()};
  // ImGui::SetNextWindowPos({extent2d.width / 4.0F, extent2d.height / 4.0F});
  // ImGui::Begin("viewport");
  // ImGui::Image(descriptor_set_, {extent2d.width / 2.0F, extent2d.height
  // / 2.0F}); ImGui::End();
}

}  // namespace luka
