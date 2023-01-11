file(GLOB_RECURSE DDLS_SOURCES "src/*.h" "src/*.cpp")

add_library(Engine SHARED ${DDLS_SOURCES})
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
    glfw)

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