macro (libut_define_target name)
    add_library(ut_${name} INTERFACE)
    target_sources(
        ut_${name}
        PUBLIC FILE_SET HEADERS #
               BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include #
               FILES ${ARGN}
    )
    if (NOT TARGET ut::${name})
        add_library(ut::${name} ALIAS ut_${name})
    endif ()

    if (NOT TARGET ut::ut_${name})
        add_library(ut::ut_${name} ALIAS ut_${name})
    endif ()
    message(STATUS "libut: adding ut::${name}")
endmacro ()
