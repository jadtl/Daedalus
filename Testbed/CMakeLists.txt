if (APPLE)
    add_executable(Testbed MACOSX_BUNDLE src/main.cpp)
    set_target_properties(Testbed PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER my.domain.style.identifier.daedalus
        MACOSX_BUNDLE_BUNDLE_NAME "Daedalus"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    )
else()
    add_executable(Testbed src/main.cpp)
endif()

target_link_libraries(Testbed Daedalus::Engine)