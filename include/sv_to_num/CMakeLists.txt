cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_sv_to_num INTERFACE)
target_include_directories(libut_target_sv_to_num INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::sv_to_num")
add_library(UT::sv_to_num ALIAS libut_target_sv_to_num)
