cmake_minimum_required(VERSION 3.24)
project(luka_engine)

file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/source_path.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated/source_path.h"
)

set(HEADER_FILES
  # platform
  platform/common.h
  # core
  core/json.h
  core/log.h
  core/math.h
  core/util.h
  # resource
  resource/asset/asset.h
  resource/asset/model.h
  resource/asset/image.h
  resource/asset/shader.h
  resource/config/config.h
  resource/config/generated/source_path.h
  # function
  function/camera/camera.h
  function/context/context.h
  function/function_input/function_input.h
  function/function_ui/function_ui.h
  function/gpu/buffer.h
  function/gpu/gpu.h
  function/gpu/image.h

  function/rendering/context.h
  function/rendering/geometry_subpass.h
  function/rendering/pass.h
  function/rendering/rendering.h
  function/rendering/spirv.h
  function/rendering/subpass.h
  function/rendering/swapchain_pass.h

  function/scene_graph/accessor.h
  function/scene_graph/buffer_view.h
  function/scene_graph/buffer.h
  function/scene_graph/camera.h
  function/scene_graph/component.h
  function/scene_graph/image.h
  function/scene_graph/light.h
  function/scene_graph/map.h
  function/scene_graph/material.h
  function/scene_graph/mesh.h
  function/scene_graph/node.h
  function/scene_graph/sampler.h
  function/scene_graph/scene_graph.h
  function/scene_graph/scene.h
  function/scene_graph/script.
  function/scene_graph/texture.h
  function/time/time.h
  function/window/window.h
  # editor
  editor/editor_input/editor_input.h
  editor/editor_ui/editor_ui.h
  # engine
  engine.h
)

set(SOURCE_FILES
  # core
  core/json.cc
  core/util.cc
  # resource
  resource/asset/asset.cc
  resource/asset/model.cc
  resource/asset/image.cc
  resource/asset/shader.cc
  resource/config/config.cc
  # function
  function/camera/camera.cc
  function/context/context.cc
  function/function_input/function_input.cc
  function/function_ui/function_ui.cc
  function/gpu/buffer.cc
  function/gpu/gpu.cc
  function/gpu/image.cc

  function/rendering/context.cc
  function/rendering/geometry_subpass.cc
  function/rendering/pass.cc
  function/rendering/rendering.cc
  function/rendering/spirv.cc
  function/rendering/subpass.cc
  function/rendering/swapchain_pass.cc

  function/scene_graph/accessor.cc
  function/scene_graph/buffer_view.cc
  function/scene_graph/buffer.cc
  function/scene_graph/camera.cc
  function/scene_graph/component.cc
  function/scene_graph/image.cc
  function/scene_graph/light.cc
  function/scene_graph/map.cc
  function/scene_graph/material.cc
  function/scene_graph/mesh.cc
  function/scene_graph/node.cc
  function/scene_graph/sampler.cc
  function/scene_graph/scene_graph.cc
  function/scene_graph/scene.cc
  function/scene_graph/texture.cc
  function/time/time.cc
  function/window/window.cc
  # editor
  editor/editor_input/editor_input.cc
  editor/editor_ui/editor_ui.cc
  # engine
  engine.cc
  main.cc
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE luka_third_party)
target_precompile_headers(${PROJECT_NAME} PRIVATE platform/pch.h)