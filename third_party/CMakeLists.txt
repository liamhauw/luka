cmake_minimum_required(VERSION 3.24)
project(luka_third_party)

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

# Vulkan SDK
find_package(Vulkan REQUIRED GLOBAL)

# glfw
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw)

# glm
add_subdirectory(glm)

# imgui
add_library(imgui)
add_library(imgui::imgui ALIAS imgui)
target_sources(imgui PRIVATE
  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_vulkan.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui.cpp
)
target_include_directories(imgui PUBLIC
  imgui
  imgui/backends
)
target_link_libraries(imgui PRIVATE
  Vulkan::Vulkan
  glfw
)

# json
set(JSON_BuildTests OFF)
set(JSON_GlobalUDLs OFF)
set(JSON_ImplicitConversions OFF)
set(JSON_Install OFF)
add_subdirectory(json)

# spdlog
add_subdirectory(spdlog)

# vma
add_subdirectory(vma SYSTEM)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE third_party.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC
  Vulkan::Vulkan
  glfw
  imgui::imgui
  glm::glm
  nlohmann_json::nlohmann_json
  spdlog::spdlog_header_only
  GPUOpen::VulkanMemoryAllocator
)

set(CMAKE_WARN_DEPRECATED TRUE CACHE BOOL "" FORCE)