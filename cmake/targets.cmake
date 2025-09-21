macro (libut_define_target name)
    add_library(${name} INTERFACE)
    target_sources(
        ${name}
        PUBLIC FILE_SET HEADERS #
               BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include #
               FILES ${ARGN}
    )

    # target_include_directories(
    #     ${name} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
    # )
    message(STATUS "libut: adding ${name}")
endmacro ()
