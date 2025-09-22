cmake_minimum_required(VERSION 3.21)
project(
    libut
    VERSION 0.1.0
    DESCRIPTION "Various useful bits of C++"
    HOMEPAGE_URL "https://github.com/dk949/libut"
    LANGUAGES CXX
)
include(cmake/install.cmake)
include(cmake/targets.cmake)

file(GLOB includes "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

add_library(ut INTERFACE)
foreach (include ${includes})
    add_subdirectory("${include}")
    cmake_path(GET include FILENAME module_name)
    target_link_libraries(ut INTERFACE ut_${module_name})
    list(APPEND LIBUT_TARGET_LIST ut_${module_name})
endforeach ()
if (NOT TARGET ut::ut)
    add_library(ut::ut ALIAS ut)
endif ()
list(APPEND LIBUT_TARGET_LIST ut)
set(LIBUT_TARGET_LIST
    "${LIBUT_TARGET_LIST}"
    CACHE INTERNAL "" FORCE
)
message(STATUS "libut: adding ut::ut")
libut_install()

if (PROJECT_IS_TOP_LEVEL)
    option(ENABLE_TESTING "Should tests be enabled?" ON)
    if (ENABLE_TESTING)

        if (ENABLE_CACHE)
            find_program(cache_bin ccache)
            if (cache_bin)
                message(STATUS "ccache found and enabled")
                set(CMAKE_CXX_COMPILER_LAUNCHER
                    ${cache_bin}
                    CACHE STRING "cmake compiler launcher"
                )
            else ()
                message(WARNING "ccache is enabled but was not found. Not using it")
            endif ()
        endif ()

        set(CMAKE_CXX_EXTENSIONS OFF)
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
        add_subdirectory(tests)
    endif ()
endif ()
