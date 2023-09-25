cmake_minimum_required(VERSION 3.24)
project(luka_engine)

set(HEADER_FILES
  resource/asset/asset.h
  resource/config/config.h
  function/function_input/function_input.h
  function/physics/physics.h
  function/rendering/rendering.h
  function/window/window.h
  function/world/world.h
  editor/editor_input/editor_input.h
  editor/scene/scene.h
  editor/ui/ui.h
  context.h
  engine.h
)

set(SOURCE_FILES
  resource/asset/asset.cc
  resource/config/config.cc
  function/function_input/function_input.cc
  function/physics/physics.cc
  function/rendering/rendering.cc
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
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)