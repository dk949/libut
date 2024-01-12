cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_pack_loops INTERFACE)
target_include_directories(libut_target_pack_loops INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::pack_loops")
add_library(UT::pack_loops ALIAS libut_target_pack_loops)
