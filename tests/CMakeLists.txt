include(compiler_warnings.cmake)
find_package(Catch2 3 REQUIRED)
include(CTest)
include(Catch)
include(CheckSourceCompiles)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(cxx_17_files
    ${CMAKE_CURRENT_SOURCE_DIR}/add_noexcept.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/check.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/constexpr_hash.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/defer.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/dirs.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/pack_loops.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/breakpoint.cpp #
)
set(cxx_20_files
    ${cxx_17_files}
    ${CMAKE_CURRENT_SOURCE_DIR}/change_observer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/copy_traits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ownptrvec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pair.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ptrvecview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spawn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/static_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sv_to_num.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trim.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/valueptr.cpp
)

set(cxx_23_files
    ${cxx_20_files} #
    ${CMAKE_CURRENT_SOURCE_DIR}/err.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/args.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/traced_error.cpp #
)

block(SCOPE_FOR VARIABLES)
if (MSVC)
    set(CMAKE_REQUIRED_FLAGS /Zc:__cplusplus /std:c++latest)
else ()
    set(CMAKE_REQUIRED_FLAGS -std=c++23)
endif ()
check_source_compiles(
    CXX
    "
        int main(void){
        static_assert(__cplusplus >= 202302L);
        }
    "
    HAS_CXX_23
)
endblock()

function (setup_target target)
    target_link_libraries(${target} PRIVATE ut)
    set_target_warnings(${target} PRIVATE)
    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
    target_precompile_headers(${target} PRIVATE <catch.hpp>)
    if (NOT MSVC)
        target_compile_options(${target} PUBLIC "-fsanitize=address" "-Og" "-g")
        target_link_options(${target} PUBLIC "-fsanitize=address" "-lg")
    else ()
        target_compile_options(${target} PUBLIC "/Zc:__cplusplus")
    endif ()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 22)
        target_compile_options(${target} PUBLIC "-Wno-c2y-extensions")
    endif ()
    catch_discover_tests(${target})
endfunction ()

add_executable(libut_tests_17 ${cxx_17_files})
add_executable(libut_tests_20 ${cxx_20_files})

target_compile_features(libut_tests_17 PUBLIC cxx_std_17)
target_compile_features(libut_tests_20 PUBLIC cxx_std_20)

setup_target(libut_tests_17)
setup_target(libut_tests_20)

if (HAS_CXX_23)
    add_executable(libut_tests_23 ${cxx_23_files})
    target_compile_features(libut_tests_23 PUBLIC cxx_std_23)
    setup_target(libut_tests_23)
    if (NOT MSVC)
        target_link_libraries(libut_tests_23 PUBLIC stdc++exp)
    endif ()
else ()
    get_filename_component(compiler_name ${CMAKE_CXX_COMPILER} NAME)
    message(
        WARNING "${compiler_name} ${CMAKE_CXX_COMPILER_VERSION} does not support C++23, tests will not run"
    )
endif ()
