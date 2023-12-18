cmake_minimum_required(VERSION 3.24)
project(luka_resource)

set(SHADER_DIR "pbr")

file(
  GLOB_RECURSE
  GLSL_SHADERS
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.conf"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.tesc"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.tese"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.geom"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.frag"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.comp"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.mesh"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.task"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.rgen"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.rint"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.rahit"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.rchit"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.rmiss"
  "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/*.rcall"
)

file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/generated")
set(SHADER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/glsl/include")
set(SPV_SHADERS "")

foreach(GLSL_SHADER ${GLSL_SHADERS})
  get_filename_component(SHADER_NAME ${GLSL_SHADER} NAME)
  set(SPV_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/asset/shader/${SHADER_DIR}/generated/${SHADER_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPV_SHADER}
    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -I${SHADER_INCLUDE_DIR} -V100 -o ${SPV_SHADER} ${GLSL_SHADER}
    DEPENDS ${GLSL_SHADER}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  list(APPEND SPV_SHADERS ${SPV_SHADER})
endforeach()

add_custom_target(${PROJECT_NAME} DEPENDS ${SPV_SHADERS} SOURCES ${GLSL_SHADERS})