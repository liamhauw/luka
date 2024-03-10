cmake_minimum_required(VERSION 3.24)
project(luka_engine)

file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/root_path.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated/root_path.h"
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
  resource/asset/scene.h
  resource/asset/shader.h
  resource/config/config.h
  resource/config/generated/source_path.h
  resource/gpu/buffer.h
  resource/gpu/gpu.h
  resource/gpu/image.h
  # resource/scene_graph/accessor.h
  # resource/scene_graph/buffer_view.h
  # resource/scene_graph/buffer.h
  # resource/scene_graph/camera.h
  # resource/scene_graph/component.h
  # resource/scene_graph/image.h
  # resource/scene_graph/light.h
  # resource/scene_graph/map.h
  # resource/scene_graph/material.h
  # resource/scene_graph/mesh.h
  # resource/scene_graph/node.h
  # resource/scene_graph/sampler.h
  # resource/scene_graph/scene_graph.h
  # resource/scene_graph/scene.h
  # resource/scene_graph/script.
  # resource/scene_graph/texture.h
  resource/time/time.h
  resource/window/window.h

  # function
  function/camera/camera.h
  function/function_input/function_input.h
  function/function_ui/function_ui.h
  function/rendering/context.h
  function/rendering/pass.h
  function/rendering/rendering.h
  function/rendering/spirv.h
  function/rendering/subpass.h
  function/rendering/swapchain_pass.h
  function/rendering/swapchain_subpass.h

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
  resource/asset/scene.cc
  resource/asset/shader.cc
  resource/config/config.cc
  resource/gpu/buffer.cc
  resource/gpu/gpu.cc
  resource/gpu/image.cc
  # resource/scene_graph/accessor.cc
  # resource/scene_graph/buffer_view.cc
  # resource/scene_graph/buffer.cc
  # resource/scene_graph/camera.cc
  # resource/scene_graph/component.cc
  # resource/scene_graph/image.cc
  # resource/scene_graph/light.cc
  # resource/scene_graph/map.cc
  # resource/scene_graph/material.cc
  # resource/scene_graph/mesh.cc
  # resource/scene_graph/node.cc
  # resource/scene_graph/sampler.cc
  # resource/scene_graph/scene_graph.cc
  # resource/scene_graph/scene.cc
  # resource/scene_graph/texture.cc
  resource/time/time.cc
  resource/window/window.cc

  # # function
  # function/camera/camera.cc
  # function/function_input/function_input.cc
  # function/function_ui/function_ui.cc
  # function/rendering/context.cc
  # function/rendering/pass.cc
  # function/rendering/rendering.cc
  # function/rendering/spirv.cc
  # function/rendering/subpass.cc
  # function/rendering/swapchain_pass.cc
  # function/rendering/swapchain_subpass.cc

  # # editor
  # editor/editor_input/editor_input.cc
  # editor/editor_ui/editor_ui.cc

  # # engine
  engine.cc
  main.cc
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE luka_third_party)
target_precompile_headers(${PROJECT_NAME} PRIVATE platform/pch.h)