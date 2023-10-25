cmake_minimum_required(VERSION 3.24)
project(luka_engine)

file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/source_path.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated/source_path.h"
)

set(HEADER_FILES
  platform/common.h
  core/log.h
  core/math.h
  core/util.h
  resource/asset/asset.h
  resource/asset/gltf.h
  resource/config/config.h
  function/function_input/function_input.h
  function/physics/physics.h
  function/rendering/common.h
  function/rendering/dynamic_buffer.h
  function/rendering/gpu.h
  function/rendering/rendering.h
  function/rendering/static_buffer.h
  function/rendering/upload_buffer.h
  function/time/time.h
  function/window/window.h
  function/world/world.h
  editor/editor_input/editor_input.h
  editor/scene/scene.h
  editor/ui/ui.h
  context.h
  engine.h
)

set(SOURCE_FILES
  core/util.cc
  resource/asset/gltf.cc
  resource/asset/asset.cc
  resource/config/generated/source_path.h
  resource/config/config.cc
  function/function_input/function_input.cc
  function/physics/physics.cc
  function/rendering/common.cc
  function/rendering/dynamic_buffer.cc
  function/rendering/gpu.cc
  function/rendering/rendering.cc
  function/rendering/static_buffer.cc
  function/rendering/upload_buffer.cc
  function/time/time.cc
  function/window/window.cc
  function/world/world.cc
  editor/editor_input/editor_input.cc
  editor/scene/scene.cc
  editor/ui/ui.cc
  context.cc
  engine.cc
  main.cc
)

add_executable(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} luka_resource)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE luka_third_party)