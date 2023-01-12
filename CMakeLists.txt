cmake_minimum_required(VERSION 3.2)

project(Daedalus VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(External/fmt)
add_subdirectory(External/glfw)
add_subdirectory(External/glm)

find_package(Vulkan REQUIRED)

if (LINUX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Compiler warnings
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings -Wconversion -Wunreachable-code -Wcast-qual -W -Wformat=2 -Winit-self -Wuninitialized)
endif()

add_compile_options(-DDDLS_ASSERT)

add_subdirectory(Engine)
add_subdirectory(Testbed)
add_subdirectory(Tests)
