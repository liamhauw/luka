/*
  SPDX license identifier: MIT

  Copyright (c) 2023 Liam Hauw

  Window header file.
*/

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace luka {

struct WindowCreateInfo {
  int width{1280};
  int height{720};
  std::string title{"luka"};
};

class Window {
 public:
  friend class FunctionInput;
  friend class EditorInput;

  using OnWindowSizeFunc = std::function<void(int, int)>;
  using OnWindowCloseFunc = std::function<void()>;
  using OnWindowIconifyFunc = std::function<void(int)>;
  using OnFramebufferSizeFunc = std::function<void(int, int)>;
  using OnKeyFunc = std::function<void(int, int, int, int)>;
  using OnCharFunc = std::function<void(unsigned)>;
  using OnCharModFunc = std::function<void(unsigned, int)>;
  using OnMouseButtonFunc = std::function<void(int, int, int)>;
  using OnCursorPosFunc = std::function<void(double, double)>;
  using OnCursorEnterFunc = std::function<void(int)>;
  using OnScrollFunc = std::function<void(double, double)>;
  using OnDropFunc = std::function<void(int, const char**)>;

  explicit Window(const WindowCreateInfo& window_create_info = {});
  ~Window();

  void Tick();

  bool GetWindowResized() const;
  void SetWindowResized(bool resized);
  void GetWindowSize(int* width, int* height) const;
  bool WindowShouldClose() const;
  void SetWindowShouldClose();
  bool GetIconified() const;
  void SetIconified(bool iconified);
  bool GetFramebufferResized() const;
  void SetFramebufferResized(bool resized);
  void GetFramebufferSize(int* width, int* height) const;
  bool GetFocusMode() const;
  void SetFocusMode(bool mode);

  static std::vector<const char*> GetRequiredInstanceExtension();
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
  static void ErrorCallback(int error_code, const char* description);
  static void WindowSizeCallback(GLFWwindow* glfw_window, int width,
                                 int height);
  static void WindowCloseCallback(GLFWwindow* glfw_window);
  static void WindowIconifyCallback(GLFWwindow* glfw_window, int iconified);
  static void FramebufferSizeCallback(GLFWwindow* glfw_window, int width,
                                      int height);
  static void KeyCallback(GLFWwindow* glfw_window, int key, int scancode,
                          int action, int mods);
  static void CharCallback(GLFWwindow* glfw_window, unsigned int codepoint);
  static void CharModCallback(GLFWwindow* glfw_window, unsigned int codepoint,
                              int mods);
  static void MouseButtonCallback(GLFWwindow* glfw_window, int button,
                                  int action, int mods);
  static void CursorPosCallback(GLFWwindow* glfw_window, double xpos,
                                double ypos);
  static void CursorEnterCallback(GLFWwindow* glfw_window, int entered);
  static void ScrollCallback(GLFWwindow* glfw_window, double xoffset,
                             double yoffset);
  static void DropCallback(GLFWwindow* glfw_window, int path_count,
                           const char** paths);

  void OnWindowClose();
  void OnWindowSize(int width, int height);
  void OnWindowIconify(int iconified);
  void OnFramebufferSize(int width, int height);
  void OnKey(int key, int scancode, int action, int mods);
  void OnChar(unsigned codepoint);
  void OnCharMod(unsigned codepoint, int mods);
  void OnMouseButton(int button, int action, int mods);
  void OnCursorPos(double xpos, double ypos);
  void OnCursorEnter(int entered);
  void OnScroll(double xoffset, double yoffset);
  void OnDrop(int path_count, const char** path);

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

  GLFWwindow* glfw_window_{nullptr};
  bool window_resized_{false};
  bool window_iconified_{false};
  bool framebuffer_resized_{false};
  bool focus_mode_{false};
  double cursor_last_xpos_{0.0};
  double cursor_last_ypos_{0.0};
  double cursor_delta_xpos_{0.0};
  double cursor_delta_ypos_{0.0};
};

}  // namespace luka
