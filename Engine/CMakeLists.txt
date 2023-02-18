file(GLOB_RECURSE DDLS_SOURCES "src/*.h" "src/*.cpp")
if (WIN32)
    add_library(Engine STATIC ${DDLS_SOURCES})
else()
    add_library(Engine SHARED ${DDLS_SOURCES})
endif()
add_library(Daedalus::Engine ALIAS Engine)

target_include_directories(Engine 
    PUBLIC 
    src/)

set_target_properties(Engine 
    PROPERTIES 
    OUTPUT_NAME "Daedalus" 
    CXX_VISIBILITY_PRESET hidden)

target_link_libraries(Engine
    fmt::fmt
    Vulkan::Vulkan
    glad
    glfw
    glm
    imgui
    stb
    lua::lib)

target_compile_definitions(Engine 
    PRIVATE
    DDLS_EXPORT
    ## Version ##
    DDLS_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
    DDLS_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
    DDLS_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
    PUBLIC
    ## Debug ##
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DEBUG DDLS_DEBUG>
    ## Platforms ##
    # Windows
	$<$<PLATFORM_ID:Windows>:DDLS_PLATFORM_WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX>
	# Linux
	$<$<PLATFORM_ID:Linux>:DDLS_PLATFORM_LINUX>
	# macOS
	$<$<PLATFORM_ID:Darwin>:DDLS_PLATFORM_MACOS>
)

set(SHADER_DIR "shaders")

file(GLOB_RECURSE GLSL_FRAG
    "${SHADER_DIR}/*.frag.glsl"
)

file(GLOB_RECURSE GLSL_VERT
    "${SHADER_DIR}/*.vert.glsl"
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

add_dependencies(Engine Shaders)
