# TODO: Add earlier minimum version
cmake_minimum_required(VERSION 3.05)

project(Daedalus VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

# Compiler warnings
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings -Wconversion -Wunreachable-code -Wcast-qual -W -Wformat=2 -Winit-self -Wuninitialized)
endif()

add_compile_options(-DDDLS_ASSERT)

add_subdirectory(Engine)
add_subdirectory(Tests)
