cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_resource INTERFACE)
target_sources(libut_target_resource PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/resource/optionalof.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/resource/resource.hpp
)
message(STATUS "libut: adding UT::resource")
add_library(UT::resource ALIAS libut_target_resource)
