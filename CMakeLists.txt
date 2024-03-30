cmake_minimum_required(VERSION 3.24)
project(luka)

add_subdirectory(third_party)
add_subdirectory(engine)

set(INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
install(TARGETS luka_engine DESTINATION ${INSTALL_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resource DESTINATION ${INSTALL_DIR})
