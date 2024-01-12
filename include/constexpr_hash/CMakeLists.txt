cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_constexpr_hash INTERFACE)
target_include_directories(libut_target_constexpr_hash INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::constexpr_hash")
add_library(UT::constexpr_hash ALIAS libut_target_constexpr_hash)
