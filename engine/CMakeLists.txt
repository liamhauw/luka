cmake_minimum_required(VERSION 3.24)
project(luka_engine)

file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/root_path.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/resource/config/generated/root_path.h"
)

file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

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

find_program(CLANG_FORMAT clang-format)

if(CLANG_FORMAT)
  add_custom_target(
    clang_format
    COMMAND ${CLANG_FORMAT} -i ${HEADER_FILES} ${SOURCE_FILES})
  add_dependencies(${PROJECT_NAME} clang_format)
endif()
