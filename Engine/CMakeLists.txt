file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp" "src/*.m")

add_library(Daedalus STATIC ${HEADERS} ${SOURCES})
add_library(Daedalus::Daedalus ALIAS Daedalus)

target_compile_features(Daedalus
    PUBLIC 
    c_std_11 
    cxx_std_17
)

target_compile_definitions(Daedalus 
    PRIVATE 
    DDLS_EXPORT
    PUBLIC
    # Daedalus version
    DDLS_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
    DDLS_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
    DDLS_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
    # Debug macro
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DEBUG DDLS_DEBUG>
    # Windows
	$<$<PLATFORM_ID:Windows>:DDLS_PLATFORM_WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX>
	# Linux
	$<$<PLATFORM_ID:Linux>:DDLS_PLATFORM_LINUX>
	# macOS
	$<$<PLATFORM_ID:Darwin>:DDLS_PLATFORM_MACOS>
)

target_include_directories(Daedalus 
    PUBLIC
    # Daedalus sources
    src
)

target_link_libraries(Daedalus 
    PUBLIC
    Vulkan::Vulkan 
    glm 
    glfw
    fmt::fmt
    tinyobjloader
    vk-bootstrap
    VulkanMemoryAllocator
)

