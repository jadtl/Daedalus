file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")

set(SHADER_DIR "../Shaders")

add_executable(Testbed ${HEADERS} ${SOURCES})
target_link_libraries(Testbed PRIVATE Daedalus::Daedalus)

target_compile_definitions(Testbed
  PRIVATE
  DDLS_EXPORT
)

file(GLOB_RECURSE GLSL_FRAG
    "${SHADER_DIR}/*.frag.glsl"
)

file(GLOB_RECURSE GLSL_VERT
    "${SHADER_DIR}*.vert.glsl"
)

foreach(GLSL ${GLSL_FRAG})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${SHADER_DIR}/${FILE_NAME}.spv")
  string(REPLACE ".glsl" "" SPIRV "${SPIRV}")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR}
    COMMAND glslc -fshader-stage=frag ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

foreach(GLSL ${GLSL_VERT})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${SHADER_DIR}/${FILE_NAME}.spv")
  string(REPLACE ".glsl" "" SPIRV "${SPIRV}")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR}
    COMMAND glslc -fshader-stage=vert ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(Testbed Shaders)

add_custom_command(TARGET Testbed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/Shaders"
        "${PROJECT_BINARY_DIR}/${SHADER_DIR}"
)