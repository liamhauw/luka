/*
  SPDX license identifier: MIT

  Copyright (C) 2023 Liam Hauw.

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

const int kDefaultWidth{1280};
const int kDefaultHeight{720};
const std::string kDefaultTitle{"luka"};

struct WindowCreateInfo {
  int width{kDefaultWidth};
  int height{kDefaultHeight};
  std::string title{kDefaultTitle};
};

class Window {
 public:
  explicit Window(const WindowCreateInfo& window_create_info = {});
  ~Window();

  void Tick();

  bool GetFocusMode() const;
  void SetFocusMode(bool mode);
  bool WindowShouldClose() const;
  void SetWindowShouldClose();

  static std::vector<const char*> GetRequiredInstanceExtension();
  void CreateWindowSurface(const vk::raii::Instance& instance,
                           VkSurfaceKHR* surface);
  void GetFramebufferSize(int* width, int* height);

  using OnWindowCloseFunc = std::function<void()>;
  using OnWindowSizeFunc = std::function<void(int, int)>;
  using OnKeyFunc = std::function<void(int, int, int, int)>;
  using OnCharFunc = std::function<void(unsigned)>;
  using OnCharModFunc = std::function<void(unsigned, int)>;
  using OnMouseButtonFunc = std::function<void(int, int, int)>;
  using OnCursorPosFunc = std::function<void(double, double)>;
  using OnCursorEnterFunc = std::function<void(int)>;
  using OnScrollFunc = std::function<void(double, double)>;
  using OnDropFunc = std::function<void(int, const char**)>;

  void RegisterOnWindowCloseFunc(const OnWindowCloseFunc& func);
  void RegisterOnWindowSizeFunc(const OnWindowSizeFunc& func);
  void RegisterOnKeyFunc(const OnKeyFunc& func);
  void RegisterOnCharFunc(const OnCharFunc& func);
  void RegisterOnCharModFunc(const OnCharModFunc& func);
  void RegisterOnMouseButtonFunc(const OnMouseButtonFunc& func);
  void RegisterOnCursorPosFunc(const OnCursorPosFunc& func);
  void RegisterOnCursorEnterFunc(const OnCursorEnterFunc& func);
  void RegisterOnScrollFunc(const OnScrollFunc& func);
  void RegisterOnDropFunc(const OnDropFunc& func);

 private:
  static void ErrorCallback(int error, const char* description);
  static void WindowCloseCallback(GLFWwindow* glfw_window);
  static void WindowSizeCallback(GLFWwindow* glfw_window, int width,
                                 int height);
  static void KeyCallback(GLFWwindow* glfw_window, int key, int scancode,
                          int action, int mod);
  static void CharCallback(GLFWwindow* glfw_window, unsigned codepoint);
  static void CharModCallback(GLFWwindow* glfw_window, unsigned codepoint,
                              int mod);
  static void MouseButtonCallback(GLFWwindow* glfw_window, int button,
                                  int action, int mod);
  static void CursorPosCallback(GLFWwindow* glfw_window, double xpos,
                                double ypos);
  static void CursorEnterCallback(GLFWwindow* glfw_window, int entered);
  static void ScrollCallback(GLFWwindow* glfw_window, double xoffset,
                             double yoffset);
  static void DropCallback(GLFWwindow* glfw_window, int count,
                           const char** paths);

  void OnWindowClose();
  void OnWindowSize(int width, int height);
  void OnKey(int key, int scancode, int action, int mod);
  void OnChar(unsigned codepoint);
  void OnCharMod(unsigned codepoint, int mod);
  void OnMouseButton(int button, int action, int mod);
  void OnCursorPos(double xpos, double ypos);
  void OnCursorEnter(int entered);
  void OnScroll(double xoffset, double yoffset);
  void OnDrop(int count, const char** path);

  int width_;
  int height_;
  std::string title_;
  GLFWwindow* glfw_window_{nullptr};

  bool focus_mode_{false};

  std::vector<OnWindowCloseFunc> on_window_close_func_;
  std::vector<OnWindowSizeFunc> on_window_size_func_;
  std::vector<OnKeyFunc> on_key_func_;
  std::vector<OnCharFunc> on_char_func_;
  std::vector<OnCharModFunc> on_char_mod_func_;
  std::vector<OnMouseButtonFunc> on_mouse_button_func_;
  std::vector<OnCursorPosFunc> on_cursor_pos_func_;
  std::vector<OnCursorEnterFunc> on_cursor_enter_func_;
  std::vector<OnScrollFunc> on_scroll_func_;
  std::vector<OnDropFunc> on_drop_func_;
};

}  // namespace luka
