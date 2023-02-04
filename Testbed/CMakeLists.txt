if (APPLE)
    file(GLOB_RECURSE RESOURCE_FILES "${PROJECT_BINARY_DIR}/Engine/shaders/*.spv")
    add_executable(Testbed MACOSX_BUNDLE src/main.cpp ${RESOURCE_FILES})
    set_target_properties(Testbed PROPERTIES
        MACOSX_BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER com.jadtl.daedalus
        MACOSX_BUNDLE_BUNDLE_NAME "Daedalus"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_FRAMEWORK_IDENTIFIER com.jadtl.daedalus
        RESOURCE "${RESOURCE_FILES}"
    )
else()
    add_executable(Testbed src/main.cpp)
endif()

if (WIN32)
    target_compile_definitions(Testbed
        PRIVATE
        DDLS_EXPORT)
endif()

target_link_libraries(Testbed Daedalus::Engine)

add_custom_command(TARGET Testbed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/Engine/shaders"
        "${PROJECT_BINARY_DIR}/Testbed")
