include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

function (libut_install)
    install(
        TARGETS ${LIBUT_TARGET_LIST}
        EXPORT utTargets
        FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT utTargets
        NAMESPACE ut::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ut
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_LIST_DIR}/cmake/utConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/utConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ut"
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/utConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/utConfig.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/utConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ut
    )
endfunction ()
