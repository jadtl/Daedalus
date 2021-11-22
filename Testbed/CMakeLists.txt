file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${TESTBED} ${HEADERS} ${SOURCES})
target_link_libraries(${TESTBED} ${ENGINE})

file(GLOB_RECURSE GLSL_FRAG
    "../Shaders/*.frag.glsl"
)

file(GLOB_RECURSE GLSL_VERT
    "../Shaders/*.vert.glsl"
)

foreach(GLSL ${GLSL_FRAG})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "../Shaders/${FILE_NAME}.spv")
  string(REPLACE ".glsl" "" SPIRV "${SPIRV}")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "../Shaders/"
    COMMAND glslc -fshader-stage=frag ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

foreach(GLSL ${GLSL_VERT})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "../Shaders/${FILE_NAME}.spv")
  string(REPLACE ".glsl" "" SPIRV "${SPIRV}")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "../Shaders/"
    COMMAND glslc -fshader-stage=vert ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(${TESTBED} Shaders)