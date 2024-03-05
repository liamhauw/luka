cmake_minimum_required(VERSION 3.24)
project(luka)
add_subdirectory(third_party)
add_subdirectory(engine)

install(TARGETS luka_engine DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resource DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)