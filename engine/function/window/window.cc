// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/window/window.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Window::Window(const WindowCreateInfo& window_ci) {
  if (!static_cast<bool>(glfwInit())) {
    THROW("Fail to init glfw.");
  }

  glfwSetErrorCallback(ErrorCallback);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfw_window_ = glfwCreateWindow(window_ci.width, window_ci.height,
                                  window_ci.title.c_str(), nullptr, nullptr);
  if (!static_cast<bool>(glfw_window_)) {
    THROW("Fail to create glfw window.");
  }

  glfwSetWindowUserPointer(glfw_window_, this);
  glfwSetWindowSizeCallback(glfw_window_, WindowSizeCallback);
  glfwSetWindowCloseCallback(glfw_window_, WindowCloseCallback);
  glfwSetWindowIconifyCallback(glfw_window_, WindowIconifyCallback);
  glfwSetFramebufferSizeCallback(glfw_window_, FramebufferSizeCallback);
  glfwSetKeyCallback(glfw_window_, KeyCallback);
  glfwSetCharCallback(glfw_window_, CharCallback);
  glfwSetCharModsCallback(glfw_window_, CharModCallback);
  glfwSetMouseButtonCallback(glfw_window_, MouseButtonCallback);
  glfwSetCursorPosCallback(glfw_window_, CursorPosCallback);
  glfwSetCursorEnterCallback(glfw_window_, CursorEnterCallback);
  glfwSetScrollCallback(glfw_window_, ScrollCallback);
  glfwSetDropCallback(glfw_window_, DropCallback);
}

Window::~Window() {
  glfwDestroyWindow(glfw_window_);
  glfwTerminate();
}

void Window::Tick() {
  f64 delta_time{gContext.time->GetDeltaTime()};
  std::string title{std::string{"luka "} +
                    std::to_string(static_cast<i32>(1.0 / delta_time)) +
                    " fps"};
  glfwSetWindowTitle(glfw_window_, title.c_str());
  glfwPollEvents();
}

bool Window::GetWindowResized() const { return window_resized_; }

void Window::SetWindowResized(bool resized) { window_resized_ = resized; }

void Window::GetWindowSize(i32* width, i32* height) const {
  glfwGetWindowSize(glfw_window_, width, height);
}

bool Window::WindowShouldClose() const {
  return static_cast<bool>(glfwWindowShouldClose(glfw_window_));
}

void Window::SetWindowShouldClose() {
  glfwSetWindowShouldClose(glfw_window_, GLFW_TRUE);
}

bool Window::GetIconified() const { return window_iconified_; }

void Window::SetIconified(bool iconified) { window_iconified_ = iconified; }

bool Window::GetFramebufferResized() const { return framebuffer_resized_; }

void Window::SetFramebufferResized(bool resized) {
  framebuffer_resized_ = resized;
}

void Window::GetFramebufferSize(i32* width, i32* height) const {
  glfwGetFramebufferSize(glfw_window_, width, height);
}

bool Window::GetFocusMode() const { return focus_mode_; }

void Window::SetFocusMode(bool mode) {
  focus_mode_ = mode;
  glfwSetInputMode(glfw_window_, GLFW_CURSOR,
                   focus_mode_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

std::vector<const char*> Window::GetRequiredInstanceExtensions() {
  u32 glfw_extension_count{0};
  const char** glfw_extensions{
      glfwGetRequiredInstanceExtensions(&glfw_extension_count)};
  std::vector<const char*> extension{glfw_extensions,
                                     glfw_extensions + glfw_extension_count};
  return extension;
}

void Window::CreateWindowSurface(const vk::raii::Instance& instance,
                                 VkSurfaceKHR* surface) {
  glfwCreateWindowSurface(static_cast<VkInstance>(*instance), glfw_window_,
                          nullptr, surface);
}

void Window::RegisterOnWindowCloseFunc(const OnWindowCloseFunc& func) {
  on_window_close_func_.push_back(func);
}
void Window::RegisterOnWindowSizeFunc(const OnWindowSizeFunc& func) {
  on_window_size_func_.push_back(func);
}
void Window::RegisterOnWindowIconifyFunc(const OnWindowIconifyFunc& func) {
  on_window_iconify_func_.push_back(func);
}
void Window::RegisterOnFramebufferSizeFunc(const OnFramebufferSizeFunc& func) {
  on_framebuffer_size_func_.push_back(func);
}
void Window::RegisterOnKeyFunc(const OnKeyFunc& func) {
  on_key_func_.push_back(func);
}
void Window::RegisterOnCharFunc(const OnCharFunc& func) {
  on_char_func_.push_back(func);
}
void Window::RegisterOnCharModFunc(const OnCharModFunc& func) {
  on_char_mod_func_.push_back(func);
}
void Window::RegisterOnMouseButtonFunc(const OnMouseButtonFunc& func) {
  on_mouse_button_func_.push_back(func);
}
void Window::RegisterOnCursorPosFunc(const OnCursorPosFunc& func) {
  on_cursor_pos_func_.push_back(func);
}
void Window::RegisterOnCursorEnterFunc(const OnCursorEnterFunc& func) {
  on_cursor_enter_func_.push_back(func);
}
void Window::RegisterOnScrollFunc(const OnScrollFunc& func) {
  on_scroll_func_.push_back(func);
}
void Window::RegisterOnDropFunc(const OnDropFunc& func) {
  on_drop_func_.push_back(func);
}

void Window::ErrorCallback(i32 error_code, const char* description) {
  LOGE("glfw error: [{} {}].", error_code, description);
}
void Window::WindowSizeCallback(GLFWwindow* glfw_window, i32 width,
                                i32 height) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnWindowSize(width, height);
  }
}
void Window::WindowCloseCallback(GLFWwindow* glfw_window) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnWindowClose();
  }
}
void Window::WindowIconifyCallback(GLFWwindow* glfw_window, i32 iconified) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnWindowIconify(iconified);
  }
}
void Window::FramebufferSizeCallback(GLFWwindow* glfw_window, i32 width,
                                     i32 height) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnFramebufferSize(width, height);
  }
}
void Window::KeyCallback(GLFWwindow* glfw_window, i32 key, i32 scancode,
                         i32 action, i32 mods) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnKey(key, scancode, action, mods);
  }
}
void Window::CharCallback(GLFWwindow* glfw_window, u32 codepoint) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnChar(codepoint);
  }
}
void Window::CharModCallback(GLFWwindow* glfw_window, u32 codepoint,
                             i32 mods) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnCharMod(codepoint, mods);
  }
}
void Window::MouseButtonCallback(GLFWwindow* glfw_window, i32 button,
                                 i32 action, i32 mods) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnMouseButton(button, action, mods);
  }
}
void Window::CursorPosCallback(GLFWwindow* glfw_window, f64 xpos, f64 ypos) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnCursorPos(xpos, ypos);
  }
}
void Window::CursorEnterCallback(GLFWwindow* glfw_window, i32 entered) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnCursorEnter(entered);
  }
}
void Window::ScrollCallback(GLFWwindow* glfw_window, f64 xoffset, f64 yoffset) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnScroll(xoffset, yoffset);
  }
}
void Window::DropCallback(GLFWwindow* glfw_window, i32 path_count,
                          const char** paths) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnDrop(path_count, paths);
  }
}
void Window::OnWindowClose() {
  for (auto& func : on_window_close_func_) {
    func();
  }
}
void Window::OnWindowSize(i32 width, i32 height) {
  for (auto& func : on_window_size_func_) {
    func(width, height);
  }
}
void Window::OnWindowIconify(i32 iconified) {
  for (auto& func : on_window_iconify_func_) {
    func(iconified);
  }
}
void Window::OnFramebufferSize(i32 width, i32 height) {
  for (auto& func : on_framebuffer_size_func_) {
    func(width, height);
  }
}
void Window::OnKey(i32 key, i32 scancode, i32 action, i32 mods) {
  for (auto& func : on_key_func_) {
    func(key, scancode, action, mods);
  }
}
void Window::OnChar(u32 codepoint) {
  for (auto& func : on_char_func_) {
    func(codepoint);
  }
}
void Window::OnCharMod(u32 codepoint, i32 mods) {
  for (auto& func : on_char_mod_func_) {
    func(codepoint, mods);
  }
}
void Window::OnMouseButton(i32 button, i32 action, i32 mods) {
  for (auto& func : on_mouse_button_func_) {
    func(button, action, mods);
  }
}
void Window::OnCursorPos(f64 xpos, f64 ypos) {
  for (auto& func : on_cursor_pos_func_) {
    func(xpos, ypos);
  }
}
void Window::OnCursorEnter(i32 entered) {
  for (auto& func : on_cursor_enter_func_) {
    func(entered);
  }
}
void Window::OnScroll(f64 xoffset, f64 yoffset) {
  for (auto& func : on_scroll_func_) {
    func(xoffset, yoffset);
  }
}
void Window::OnDrop(i32 path_count, const char** paths) {
  for (auto& func : on_drop_func_) {
    func(path_count, paths);
  }
}

}  // namespace luka
