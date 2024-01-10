cmake_minimum_required(VERSION 3.24)
project(luka_engine)

file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/source_path.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated/source_path.h"
)

set(HEADER_FILES
  platform/common.h
  core/json.h
  core/log.h
  core/math.h
  core/util.h
  resource/asset/asset.h
  resource/asset/model.h
  resource/asset/image.h
  resource/config/config.h
  resource/context/context.h
  function/camera/camera.h
  function/function_input/function_input.h
  function/function_ui/function_ui.h
  function/gpu/buffer.h
  function/gpu/gpu.h
  function/gpu/image.h
  function/physics/physics.h
  function/rendering/frame.h
  function/rendering/pipeline.h
  function/rendering/rd_context.h
  function/rendering/rendering.h
  function/rendering/target.h
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
  function/world/world.h
  editor/editor_input/editor_input.h
  editor/editor_ui/editor_ui.h
  editor/scene/scene.h
  engine.h
)

set(SOURCE_FILES
  core/json.cc
  core/util.cc
  resource/asset/asset.cc
  resource/asset/model.cc
  resource/asset/image.cc
  resource/config/generated/source_path.h
  resource/config/config.cc
  resource/context/context.cc
  function/camera/camera.cc
  function/function_input/function_input.cc
  function/function_ui/function_ui.cc
  function/gpu/buffer.cc
  function/gpu/gpu.cc
  function/gpu/image.cc
  function/physics/physics.cc
  function/rendering/frame.cc
  function/rendering/pipeline.cc
  function/rendering/rd_context.cc
  function/rendering/rendering.cc
  function/rendering/target.cc
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
  function/world/world.cc
  editor/editor_input/editor_input.cc
  editor/editor_ui/editor_ui.cc
  editor/scene/scene.cc
  engine.cc
  main.cc
)

add_executable(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} luka_resource)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE luka_third_party)
target_precompile_headers(${PROJECT_NAME} PRIVATE platform/pch.h)

if(WIN32)
  set(DLL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ktx_4.3.0/windows/bin/ktx.dll)
  foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${DLL_FILE}
      $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
  endforeach()
endif()
