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

  ImGui_ImplVulkan_InitInfo init_info{
      gContext.gpu->GetVulkanInitInfoForImgui()};
  ImGui_ImplVulkan_Init(&init_info, gContext.gpu->GetRenderPassForImGui());

  vk::ImageCreateInfo image_ci{
      {},
      vk::ImageType::e2D,
      vk::Format::eR32G32B32A32Sfloat,
      {1, 1, 1},
      1,
      1,
      vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      vk::SharingMode::eExclusive,
      {},
      vk::ImageLayout::eUndefined};
  image_ =
      gpu_->CreateImage(image_ci, vk::ImageLayout::eShaderReadOnlyOptimal,
                        image_data_.size() * sizeof(float), image_data_.data());

  const vk::DescriptorImageInfo& descriptor_image_info{
      image_.GetDescriptorImageInfo()};

  descriptor_set_ = ImGui_ImplVulkan_AddTexture(
      descriptor_image_info.sampler, descriptor_image_info.imageView,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

FunctionUI::~FunctionUI() {
  gpu_->WaitIdle();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void FunctionUI::Tick() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("setting");
  ImGui::ColorEdit3("Color", image_data_.data());
  ImGui::End();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("viewport");
  ImGui::Image(descriptor_set_, ImGui::GetContentRegionAvail());
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::Render();
}

}  // namespace luka
