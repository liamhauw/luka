cmake_minimum_required(VERSION 3.24)
project(luka_third_party)

# Vulkan SDK
find_package(Vulkan REQUIRED GLOBAL)

# enkits
set(ENKITS_BUILD_C_INTERFACE OFF CACHE INTERNAL "" FORCE)
set(ENKITS_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
add_subdirectory(enkits)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(glfw)

# glm
set(GLM_ENABLE_CXX_20 ON CACHE INTERNAL "" FORCE)
set(GLM_DISABLE_AUTO_DETECTION ON CACHE INTERNAL "" FORCE)
add_subdirectory(glm)

# glslang
add_compile_options(-Wno-deprecated-declarations)
set(BUILD_EXTERNAL OFF CACHE INTERNAL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE INTERNAL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "" FORCE)
set(ENABLE_GLSLANG_JS OFF CACHE INTERNAL "" FORCE)
set(ENABLE_OPT OFF CACHE INTERNAL "" FORCE)
add_subdirectory(glslang)
add_compile_options(-Wdeprecated-declarations)

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
target_compile_features(imgui PRIVATE cxx_std_20)
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
target_compile_features(imgui PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PUBLIC
  Vulkan::Vulkan
  enkiTS
  glfw
  glm
  glslang glslang-default-resource-limits SPIRV 
  imgui
  nlohmann_json
  spdlog_header_only
  spirv-cross-glsl
  stb
  tinygltf
  VulkanMemoryAllocator
)