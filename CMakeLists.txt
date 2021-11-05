cmake_minimum_required(VERSION 3.16)

set(ENGINE Daedalus)

project(ENGINE)

find_package(Vulkan)

add_subdirectory(External/boost)
add_subdirectory(External/glm)
add_subdirectory(External/glfw)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNOMINMAX -DUNICODE")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -DNOMINMAX -DUNICODE")

if(WIN32)
    file(GLOB_RECURSE SOURCES Daedalus/*.cpp Windows/*.cpp)
    file(GLOB_RECURSE HEADERS Daedalus/*.h Windows/*.h)
endif()

include_directories(Daedalus)

add_executable(${ENGINE} ${HEADERS} ${SOURCES})

target_link_libraries(${ENGINE} Vulkan::Vulkan glm Boost::filesystem glfw)

file(GLOB_RECURSE GLSL_FRAG
    "Shaders/*.frag.glsl"
)

file(GLOB_RECURSE GLSL_VERT
    "Shaders/*.vert.glsl"
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

add_dependencies(${ENGINE} Shaders)