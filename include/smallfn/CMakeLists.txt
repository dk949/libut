cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_smallfn INTERFACE)
target_include_directories(libut_target_smallfn INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::smallfn")
add_library(UT::smallfn ALIAS libut_target_smallfn)
