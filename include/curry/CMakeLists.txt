cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_curry INTERFACE)
target_include_directories(libut_target_curry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::curry")
add_library(UT::curry ALIAS libut_target_curry)
