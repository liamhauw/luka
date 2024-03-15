cmake_minimum_required(VERSION 3.24)
project(luka_engine)

file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/root_path.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated/root_path.h"
)

set(HEADER_FILES

  # core
  core/json.h
  core/log.h
  core/math.h
  core/util.h

  # editor
  editor/editor_input/editor_input.h
  editor/editor_ui/editor_ui.h

  # function
  function/camera/camera.h
  function/function_input/function_input.h
  function/function_ui/function_ui.h
  function/rendering/pass.h
  function/rendering/rendering.h
  function/rendering/spirv.h
  function/rendering/subpass.h
  function/rendering/swapchain_pass.h
  function/rendering/swapchain_subpass.h

  # platform
  platform/common.h

  # resource
  resource/asset/scene_component/accessor.h
  resource/asset/scene_component/buffer_view.h
  resource/asset/scene_component/buffer.h
  resource/asset/scene_component/camera.h
  resource/asset/scene_component/component.h
  resource/asset/scene_component/image.h
  resource/asset/scene_component/light.h
  resource/asset/scene_component/material.h
  resource/asset/scene_component/mesh.h
  resource/asset/scene_component/node.h
  resource/asset/scene_component/sampler.h
  resource/asset/scene_component/scene.h
  resource/asset/scene_component/script.
  resource/asset/scene_component/texture.h
  resource/asset/asset.h
  resource/asset/frame_graph.h
  resource/asset/scene.h
  resource/asset/shader.h
  resource/config/config.h
  resource/config/generated/source_path.h
  resource/gpu/buffer.h
  resource/gpu/gpu.h
  resource/gpu/image.h
  resource/task_scheduler/task_scheduler.h
  resource/time/time.h
  resource/window/window.h

  # engine
  engine.h
)

set(SOURCE_FILES

  # core
  core/util.cc

  # resource
  resource/asset/scene_component/accessor.cc
  resource/asset/scene_component/buffer_view.cc
  resource/asset/scene_component/buffer.cc
  resource/asset/scene_component/camera.cc
  resource/asset/scene_component/component.cc
  resource/asset/scene_component/image.cc
  resource/asset/scene_component/light.cc
  resource/asset/scene_component/material.cc
  resource/asset/scene_component/mesh.cc
  resource/asset/scene_component/node.cc
  resource/asset/scene_component/sampler.cc
  resource/asset/scene_component/scene.cc
  resource/asset/scene_component/texture.cc
  resource/asset/asset.cc
  resource/asset/frame_graph.cc
  resource/asset/scene.cc
  resource/asset/shader.cc
  resource/config/config.cc
  resource/gpu/buffer.cc
  resource/gpu/gpu.cc
  resource/gpu/image.cc
  resource/task_scheduler/task_scheduler.cc
  resource/time/time.cc
  resource/window/window.cc

  # function
  function/camera/camera.cc
  function/function_input/function_input.cc
  function/function_ui/function_ui.cc
  function/rendering/pass.cc
  function/rendering/rendering.cc
  function/rendering/spirv.cc
  function/rendering/subpass.cc
  function/rendering/swapchain_pass.cc
  function/rendering/swapchain_subpass.cc

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

if(APPLE)
  set_property(
    TARGET ${PROJECT_NAME}
    PROPERTY INSTALL_RPATH
    "/usr/local/lib")
endif()