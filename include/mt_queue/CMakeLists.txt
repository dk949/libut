cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_mt_queue INTERFACE)
target_sources(libut_target_mt_queue PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/mt_queue/mt_queue.hpp
)

message(STATUS "libut: adding UT::mt_queue")
add_library(UT::mt_queue ALIAS libut_target_mt_queue)
