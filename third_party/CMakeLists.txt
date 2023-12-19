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

# ktx
if(WIN32)
  find_package(Ktx REQUIRED PATHS ktx_4.3.0/windows)
elseif(APPLE)
  find_package(Ktx REQUIRED PATHS ktx_4.3.0/macos)
else()
  message(FATAL_ERROR "Unsupported platform for ktx library.")
endif()

# spdlog
add_subdirectory(spdlog)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# tinygltf
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(tinygltf)

# vma
add_subdirectory(vma SYSTEM)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE third_party.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC
  Vulkan::Vulkan
  glfw
  glm
  imgui
  nlohmann_json
  KTX::ktx
  spdlog_header_only
  stb
  tinygltf
  VulkanMemoryAllocator
)

set(CMAKE_WARN_DEPRECATED TRUE CACHE BOOL "" FORCE)