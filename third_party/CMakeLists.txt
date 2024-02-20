cmake_minimum_required(VERSION 3.24)
project(luka_third_party)

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

# Vulkan SDK
find_package(Vulkan REQUIRED GLOBAL)

# ctpl
add_library(ctpl INTERFACE)
target_include_directories(ctpl INTERFACE ctpl)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(glfw)

# glm
set(GLM_ENABLE_CXX_20 ON CACHE INTERNAL "" FORCE)
set(GLM_DISABLE_AUTO_DETECTION ON CACHE INTERNAL "" FORCE)
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
set(JSON_BuildTests OFF CACHE INTERNAL "" FORCE)
set(JSON_GlobalUDLs OFF CACHE INTERNAL "" FORCE)
set(JSON_ImplicitConversions OFF CACHE INTERNAL "" FORCE)
set(JSON_Install OFF CACHE INTERNAL "" FORCE)
add_subdirectory(json)

# shaderc
set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
add_subdirectory(shaderc)

# spdlog
add_subdirectory(spdlog)

# spirv cross
set(SPIRV_CROSS_CLI OFF CACHE INTERNAL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE INTERNAL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE INTERNAL "" FORCE)
add_subdirectory(spirv-cross)

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
  ctpl
  glfw
  glm
  imgui
  nlohmann_json
  spdlog_header_only
  spirv-cross-glsl
  stb
  tinygltf
  VulkanMemoryAllocator
)

set(CMAKE_WARN_DEPRECATED TRUE CACHE BOOL "" FORCE)