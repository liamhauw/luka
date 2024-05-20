// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/function_ui/function_ui.h"

#include <vector>

namespace luka {

FunctionUi::FunctionUi(std::shared_ptr<Window> window, std::shared_ptr<Gpu> gpu)
    : window_{std::move(window)}, gpu_{std::move(gpu)} {
  CreateSwapchain();
  CreateImgui();
}

FunctionUi::~FunctionUi() { DestroyImgui(); }

void FunctionUi::Tick() {
  if (window_->GetIconified()) {
    return;
  }

  if (window_->GetFramebufferResized()) {
    Resize();
  }

  UpdateImgui();
  CreateUi();
}

void FunctionUi::Render(const vk::raii::CommandBuffer& command_buffer) {
  ImGui::Render();
  ImDrawData* draw_data{ImGui::GetDrawData()};
  ImGui_ImplVulkan_RenderDrawData(
      draw_data, static_cast<VkCommandBuffer>(*command_buffer));
}

void FunctionUi::CreateSwapchain() {
  // Clear
  swapchain_.clear();

  // Image count.
  const vk::SurfaceCapabilitiesKHR& surface_capabilities{
      gpu_->GetSurfaceCapabilities()};
  swapchain_info_.image_count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      swapchain_info_.image_count > surface_capabilities.maxImageCount) {
    swapchain_info_.image_count = surface_capabilities.maxImageCount;
  }

  // Format and color space.
  const std::vector<vk::SurfaceFormatKHR>& surface_formats{
      gpu_->GetSurfaceFormats()};

  vk::SurfaceFormatKHR picked_format{surface_formats[0]};

  std::vector<vk::Format> requested_formats{vk::Format::eR8G8B8A8Srgb,
                                            vk::Format::eB8G8R8A8Srgb};
  vk::ColorSpaceKHR requested_color_space{vk::ColorSpaceKHR::eSrgbNonlinear};
  for (const auto& requested_format : requested_formats) {
    auto it{std::find_if(surface_formats.begin(), surface_formats.end(),
                         [requested_format, requested_color_space](
                             const vk::SurfaceFormatKHR& f) {
                           return (f.format == requested_format) &&
                                  (f.colorSpace == requested_color_space);
                         })};
    if (it != surface_formats.end()) {
      picked_format = *it;
      break;
    }
  }

  swapchain_info_.color_format = picked_format.format;
  swapchain_info_.color_space = picked_format.colorSpace;

  // Extent.
  if (surface_capabilities.currentExtent.width ==
      std::numeric_limits<u32>::max()) {
    i32 width{};
    i32 height{};
    window_->GetFramebufferSize(&width, &height);

    swapchain_info_.extent.width = std::clamp(
        static_cast<u32>(width), surface_capabilities.minImageExtent.width,
        surface_capabilities.maxImageExtent.width);
    swapchain_info_.extent.height = std::clamp(
        static_cast<u32>(height), surface_capabilities.minImageExtent.height,
        surface_capabilities.maxImageExtent.height);
  } else {
    swapchain_info_.extent = surface_capabilities.currentExtent;
  }

  // Present mode.
  const std::vector<vk::PresentModeKHR>& present_modes{
      gpu_->GetSurfacePresentModes()};

  std::vector<vk::PresentModeKHR> requested_present_modes{
      vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate,
      vk::PresentModeKHR::eFifo};

  vk::PresentModeKHR picked_mode{vk::PresentModeKHR::eFifo};
  for (const auto& requested_present_mode : requested_present_modes) {
    auto it{std::find_if(present_modes.begin(), present_modes.end(),
                         [requested_present_mode](const vk::PresentModeKHR& p) {
                           return p == requested_present_mode;
                         })};
    if (it != present_modes.end()) {
      picked_mode = *it;
      break;
    }
  }
  swapchain_info_.present_mode = picked_mode;

  // Create swapchain
  vk::SwapchainCreateInfoKHR swapchain_ci{
      {},
      {},
      swapchain_info_.image_count,
      swapchain_info_.color_format,
      swapchain_info_.color_space,
      swapchain_info_.extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {},
      surface_capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      swapchain_info_.present_mode,
      VK_TRUE,
      {}};

  u32 graphics_queue_index{gpu_->GetGraphicsQueueIndex()};
  u32 present_queue_index{gpu_->GetPresentQueueIndex()};

  if (graphics_queue_index != present_queue_index) {
    std::vector<u32> queue_family_indices{graphics_queue_index,
                                          present_queue_index};
    swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queue_family_indices.data();
  }

  swapchain_ = gpu_->CreateSwapchain(swapchain_ci);
}

void FunctionUi::CreateImgui() {
  // Ui render pass.
  std::vector<vk::AttachmentDescription> attachment_descriptions;

  attachment_descriptions.emplace_back(
      vk::AttachmentDescriptionFlags{}, swapchain_info_.color_format,
      vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined,
      vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference color_attachment_refs{
      0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpass_description{
      {}, vk::PipelineBindPoint::eGraphics, {}, color_attachment_refs};

  vk::SubpassDependency subpass_dependency{
      VK_SUBPASS_EXTERNAL,
      0,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::AccessFlagBits::eNone,
      vk::AccessFlagBits::eColorAttachmentRead |
          vk::AccessFlagBits::eColorAttachmentWrite,
      vk::DependencyFlagBits::eByRegion};

  vk::RenderPassCreateInfo render_pass_ci{
      {}, attachment_descriptions, subpass_description, subpass_dependency};

  ui_render_pass_ = gpu_->CreateRenderPass(render_pass_ci, "ui");

  // ImGui.
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;
  io.FontGlobalScale = 1.3;

  ImGui_ImplGlfw_InitForVulkan(window_->GetGlfwWindow(), true);

  ImGui_ImplVulkan_InitInfo init_info{gpu_->GetImguiVulkanInitInfo()};
  init_info.RenderPass = *ui_render_pass_;

  ImGui_ImplVulkan_Init(&init_info);
}

void FunctionUi::DestroyImgui() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void FunctionUi::Resize() {
  gpu_->WaitIdle();
  CreateSwapchain();
}

void FunctionUi::UpdateImgui() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void FunctionUi::CreateUi() {}

const SwapchainInfo& FunctionUi::GetSwapchainInfo() const {
  return swapchain_info_;
}

const vk::raii::SwapchainKHR& FunctionUi::GetSwapchain() const {
  return swapchain_;
}

vk::raii::RenderPass FunctionUi::GetUiRenderPass() {
  return std::move(ui_render_pass_);
}

}  // namespace luka
