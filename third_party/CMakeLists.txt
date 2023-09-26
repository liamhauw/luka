cmake_minimum_required(VERSION 3.24)
project(luka_third_party)

# Vulkan SDK
find_package(Vulkan REQUIRED GLOBAL)

# glfw
set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw)

# spdlog
add_subdirectory(spdlog)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE
  Vulkan::Vulkan
  glfw
  spdlog_header_only
)