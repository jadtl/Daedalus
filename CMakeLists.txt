cmake_minimum_required(VERSION 3.2)

project(Daedalus VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(External/fmt)
add_subdirectory(External/glad)
add_subdirectory(External/glfw)
add_subdirectory(External/glm)
add_subdirectory(External/imgui)
add_subdirectory(External/stb)

find_package(Vulkan REQUIRED)
find_package(Lua REQUIRED)

if (LINUX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Compiler warnings
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings -Wconversion -Wunreachable-code -Wcast-qual -W -Wformat=2 -Winit-self -Wuninitialized)
endif()

add_compile_options(-DDDLS_ASSERT -D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(Engine)
add_subdirectory(Testbed)
add_subdirectory(Tests)
