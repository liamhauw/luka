cmake_minimum_required(VERSION 3.24)
project(luka_engine)

set(HEADER_FILES
  resource/asset/asset.h
  resource/config/config.h
  engine.h
)

set(SOURCE_FILES
  resource/asset/asset.cc
  resource/config/config.cc
  engine.cc
  main.cc
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)