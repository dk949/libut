cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_ptr_containers INTERFACE)
target_include_directories(libut_target_ptr_containers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::ptr_containers")
add_library(UT::ptr_containers ALIAS libut_target_ptr_containers)
