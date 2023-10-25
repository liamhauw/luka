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

# json
set(JSON_BuildTests OFF)
set(JSON_GlobalUDLs OFF)
set(JSON_ImplicitConversions OFF)
set(JSON_Install OFF)
add_subdirectory(json)

# spdlog
add_subdirectory(spdlog)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# tinyobjloader
add_subdirectory(tinyobjloader)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE
  Vulkan::Vulkan
  glfw
  glm
  nlohmann_json
  spdlog_header_only
  stb
  tinyobjloader
)

set(CMAKE_WARN_DEPRECATED TRUE CACHE BOOL "" FORCE)