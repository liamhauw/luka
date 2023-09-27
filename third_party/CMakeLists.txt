cmake_minimum_required(VERSION 3.24)
project(luka_third_party)

# Vulkan SDK
find_package(Vulkan REQUIRED GLOBAL)

# glfw
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw)

# spdlog
add_subdirectory(spdlog)

# tinygltf
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
set(TINYGLTF_HEADER_ONLY ON)
set(TINYGLTF_INSTALL OFF)
add_subdirectory(tinygltf)

# json
set(JSON_BuildTests OFF)
set(JSON_GlobalUDLs OFF)
set(JSON_ImplicitConversions OFF)
set(JSON_Install OFF)
add_subdirectory(json)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE
  Vulkan::Vulkan
  glfw
  spdlog::spdlog_header_only
  tinygltf
  nlohmann_json::nlohmann_json
)