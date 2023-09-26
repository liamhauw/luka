/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

  Window source file.
*/

#include "function/window/window.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Window::Window(const WindowCreateInfo& window_create_info)
    : width_{window_create_info.width},
      height_{window_create_info.height},
      title_{window_create_info.title} {
  if (!static_cast<bool>(glfwInit())) {
    THROW("Fail to init glfw.");
  }

  glfwSetErrorCallback(ErrorCallback);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfw_window_ =
      glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
  if (!static_cast<bool>(glfw_window_)) {
    THROW("Fail to create glfw window.");
  }

  glfwSetWindowUserPointer(glfw_window_, this);
  glfwSetWindowCloseCallback(glfw_window_, WindowCloseCallback);
  glfwSetWindowSizeCallback(glfw_window_, WindowSizeCallback);
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
  double delta_time{gContext.time->GetDeltaTime()};
  std::string title{std::string{"luka "} +
                    std::to_string(static_cast<int>(1.0 / delta_time)) +
                    " fps"};
  glfwSetWindowTitle(glfw_window_, title.c_str());
  glfwPollEvents();
}

bool Window::GetFocusMode() const { return focus_mode_; }

void Window::SetFocusMode(bool mode) {
  focus_mode_ = mode;
  glfwSetInputMode(glfw_window_, GLFW_CURSOR,
                   focus_mode_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool Window::WindowShouldClose() const {
  return static_cast<bool>(glfwWindowShouldClose(glfw_window_));
}

void Window::SetWindowShouldClose() {
  glfwSetWindowShouldClose(glfw_window_, GLFW_TRUE);
}

std::vector<const char*> Window::GetRequiredInstanceExtension() {
  uint32_t glfw_extension_count{0};
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

void Window::GetFramebufferSize(int* width, int* height) {
  glfwGetFramebufferSize(glfw_window_, width, height);
}

void Window::RegisterOnWindowCloseFunc(const OnWindowCloseFunc& func) {
  on_window_close_func_.push_back(func);
}
void Window::RegisterOnWindowSizeFunc(const OnWindowSizeFunc& func) {
  on_window_size_func_.push_back(func);
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

void Window::ErrorCallback(int error, const char* description) {
  LOGE("glfw error: {} {}", error, description);
}
void Window::WindowCloseCallback(GLFWwindow* glfw_window) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnWindowClose();
  }
}
void Window::WindowSizeCallback(GLFWwindow* glfw_window, int width,
                                int height) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnWindowSize(width, height);
    window->width_ = width;
    window->height_ = height;
  }
}
void Window::KeyCallback(GLFWwindow* glfw_window, int key, int scancode,
                         int action, int mods) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnKey(key, scancode, action, mods);
  }
}
void Window::CharCallback(GLFWwindow* glfw_window, unsigned codepoint) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnChar(codepoint);
  }
}
void Window::CharModCallback(GLFWwindow* glfw_window, unsigned codepoint,
                             int mod) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnCharMod(codepoint, mod);
  }
}
void Window::MouseButtonCallback(GLFWwindow* glfw_window, int button,
                                 int action, int mod) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnMouseButton(button, action, mod);
  }
}
void Window::CursorPosCallback(GLFWwindow* glfw_window, double xpos,
                               double ypos) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnCursorPos(xpos, ypos);
  }
}
void Window::CursorEnterCallback(GLFWwindow* glfw_window, int entered) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnCursorEnter(entered);
  }
}
void Window::ScrollCallback(GLFWwindow* glfw_window, double xoffset,
                            double yoffset) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnScroll(xoffset, yoffset);
  }
}
void Window::DropCallback(GLFWwindow* glfw_window, int count,
                          const char** paths) {
  if (auto* window{
          reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window))}) {
    window->OnDrop(count, paths);
  }
}

void Window::OnWindowClose() {
  for (auto& func : on_window_close_func_) {
    func();
  }
}
void Window::OnWindowSize(int width, int height) {
  for (auto& func : on_window_size_func_) {
    func(width, height);
  }
}
void Window::OnKey(int key, int scancode, int action, int mods) {
  for (auto& func : on_key_func_) {
    func(key, scancode, action, mods);
  }
}
void Window::OnChar(unsigned codepoint) {
  for (auto& func : on_char_func_) {
    func(codepoint);
  }
}
void Window::OnCharMod(unsigned codepoint, int mod) {
  for (auto& func : on_char_mod_func_) {
    func(codepoint, mod);
  }
}
void Window::OnMouseButton(int button, int action, int mod) {
  for (auto& func : on_mouse_button_func_) {
    func(button, action, mod);
  }
}
void Window::OnCursorPos(double xpos, double ypos) {
  for (auto& func : on_cursor_pos_func_) {
    func(xpos, ypos);
  }
}
void Window::OnCursorEnter(int entered) {
  for (auto& func : on_cursor_enter_func_) {
    func(entered);
  }
}
void Window::OnScroll(double xoffset, double yoffset) {
  for (auto& func : on_scroll_func_) {
    func(xoffset, yoffset);
  }
}
void Window::OnDrop(int count, const char** paths) {
  for (auto& func : on_drop_func_) {
    func(count, paths);
  }
}

}  // namespace luka
