// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// clang-format on

#include "function/time/time.h"

namespace luka {

struct WindowInfo {
  i32 width{2560};
  i32 height{1440};
  std::string title{"luka"};
};

class Window {
 public:
  friend class FunctionInput;
  friend class EditorInput;

  using OnWindowSizeFunc = std::function<void(i32, i32)>;
  using OnWindowCloseFunc = std::function<void()>;
  using OnWindowIconifyFunc = std::function<void(i32)>;
  using OnFramebufferSizeFunc = std::function<void(i32, i32)>;
  using OnKeyFunc = std::function<void(i32, i32, i32, i32)>;
  using OnCharFunc = std::function<void(u32)>;
  using OnCharModFunc = std::function<void(u32, i32)>;
  using OnMouseButtonFunc = std::function<void(i32, i32, i32)>;
  using OnCursorPosFunc = std::function<void(f64, f64)>;
  using OnCursorEnterFunc = std::function<void(i32)>;
  using OnScrollFunc = std::function<void(f64, f64)>;
  using OnDropFunc = std::function<void(i32, const char**)>;

  Window(const WindowInfo& window_info = {});
  ~Window();

  void Tick();

  GLFWwindow* GetGlfwWindow() const;
  bool GetWindowResized() const;
  void SetWindowResized(bool resized);
  void GetWindowSize(i32* width, i32* height) const;
  f32 GetWindowRatio() const;
  bool WindowShouldClose() const;
  void SetWindowShouldClose();
  bool GetIconified() const;
  void SetIconified(bool iconified);
  bool GetFramebufferResized() const;
  void SetFramebufferResized(bool resized);
  void GetFramebufferSize(i32* width, i32* height) const;
  bool GetFocusMode() const;
  void SetFocusMode(bool mode);
  bool IsMouseButtonDown(int button) const;
  bool IsMouseButtonRelease(int button) const;
  void SetCursorPos(f64 xpos, f64 ypos);

  static std::vector<const char*> GetRequiredInstanceExtensions();
  void CreateWindowSurface(const vk::raii::Instance& instance,
                           VkSurfaceKHR* surface);

  void RegisterOnWindowSizeFunc(const OnWindowSizeFunc& func);
  void RegisterOnWindowCloseFunc(const OnWindowCloseFunc& func);
  void RegisterOnWindowIconifyFunc(const OnWindowIconifyFunc& func);
  void RegisterOnFramebufferSizeFunc(const OnFramebufferSizeFunc& func);
  void RegisterOnKeyFunc(const OnKeyFunc& func);
  void RegisterOnCharFunc(const OnCharFunc& func);
  void RegisterOnCharModFunc(const OnCharModFunc& func);
  void RegisterOnMouseButtonFunc(const OnMouseButtonFunc& func);
  void RegisterOnCursorPosFunc(const OnCursorPosFunc& func);
  void RegisterOnCursorEnterFunc(const OnCursorEnterFunc& func);
  void RegisterOnScrollFunc(const OnScrollFunc& func);
  void RegisterOnDropFunc(const OnDropFunc& func);

 private:
  static void ErrorCallback(i32 error_code, const char* description);
  static void WindowSizeCallback(GLFWwindow* glfw_window, i32 width,
                                 i32 height);
  static void WindowCloseCallback(GLFWwindow* glfw_window);
  static void WindowIconifyCallback(GLFWwindow* glfw_window, i32 iconified);
  static void FramebufferSizeCallback(GLFWwindow* glfw_window, i32 width,
                                      i32 height);
  static void KeyCallback(GLFWwindow* glfw_window, i32 key, i32 scancode,
                          i32 action, i32 mods);
  static void CharCallback(GLFWwindow* glfw_window, u32 codepoint);
  static void CharModCallback(GLFWwindow* glfw_window, u32 codepoint, i32 mods);
  static void MouseButtonCallback(GLFWwindow* glfw_window, i32 button,
                                  i32 action, i32 mods);
  static void CursorPosCallback(GLFWwindow* glfw_window, f64 xpos, f64 ypos);
  static void CursorEnterCallback(GLFWwindow* glfw_window, i32 entered);
  static void ScrollCallback(GLFWwindow* glfw_window, f64 xoffset, f64 yoffset);
  static void DropCallback(GLFWwindow* glfw_window, i32 path_count,
                           const char** paths);

  void OnWindowClose();
  void OnWindowSize(i32 width, i32 height);
  void OnWindowIconify(i32 iconified);
  void OnFramebufferSize(i32 width, i32 height);
  void OnKey(i32 key, i32 scancode, i32 action, i32 mods);
  void OnChar(u32 codepoint);
  void OnCharMod(u32 codepoint, i32 mods);
  void OnMouseButton(i32 button, i32 action, i32 mods);
  void OnCursorPos(f64 xpos, f64 ypos);
  void OnCursorEnter(i32 entered);
  void OnScroll(f64 xoffset, f64 yoffset);
  void OnDrop(i32 path_count, const char** path);

  std::vector<OnWindowCloseFunc> on_window_close_func_;
  std::vector<OnWindowSizeFunc> on_window_size_func_;
  std::vector<OnWindowIconifyFunc> on_window_iconify_func_;
  std::vector<OnFramebufferSizeFunc> on_framebuffer_size_func_;
  std::vector<OnKeyFunc> on_key_func_;
  std::vector<OnCharFunc> on_char_func_;
  std::vector<OnCharModFunc> on_char_mod_func_;
  std::vector<OnMouseButtonFunc> on_mouse_button_func_;
  std::vector<OnCursorPosFunc> on_cursor_pos_func_;
  std::vector<OnCursorEnterFunc> on_cursor_enter_func_;
  std::vector<OnScrollFunc> on_scroll_func_;
  std::vector<OnDropFunc> on_drop_func_;

  void WindowSize(i32 width, i32 height);
  void WindowIconify(i32 iconified);
  void FramebufferSize(i32 width, i32 height);

  GLFWwindow* glfw_window_{nullptr};
  bool window_resized_{false};
  bool window_iconified_{false};
  bool framebuffer_resized_{false};
  bool focus_mode_{false};
};

}  // namespace luka
