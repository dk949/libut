cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_check INTERFACE)
target_include_directories(libut_target_check INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::check")
add_library(UT::check ALIAS libut_target_check)
