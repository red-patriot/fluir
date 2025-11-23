include(GNUInstallDirs)

function (configure_cpack)
    set(CPACK_PACKAGE_NAME fluir)
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_CONTACT "bltanner105+fluir@gmail.com")
    set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libfmt9, libtinyxml2-10")

    include(CPack)
endfunction ()

function (setup_fluir_install)
    install(
        DIRECTORY artifacts/Launcher/
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        USE_SOURCE_PERMISSIONS
    )
    install(
        DIRECTORY artifacts/Editor-BE/
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/fluir
        USE_SOURCE_PERMISSIONS
    )
    install(
        DIRECTORY artifacts/Editor-FE/linux-unpacked/
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/fluir/editor-fe
        USE_SOURCE_PERMISSIONS
    )
    install(TARGETS fluir.compiler fluir.vm
            DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/fluir
    )
    install(FILES launcher/fluir-config.yaml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/fluir
    )

    configure_file(
        "${CMAKE_SOURCE_DIR}/packaging/debian/postinst.in"
        "${CMAKE_SOURCE_DIR}/packaging/debian/postinst"
        @ONLY
    )
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_SOURCE_DIR}/packaging/debian/postinst"
    )

    configure_cpack()
endfunction ()
