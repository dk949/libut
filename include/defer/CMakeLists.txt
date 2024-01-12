cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_defer INTERFACE)
target_include_directories(libut_target_defer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::defer")
add_library(UT::defer ALIAS libut_target_defer)
