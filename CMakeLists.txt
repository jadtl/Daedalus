cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)
project(Daedalus VERSION 0.0.1 LANGUAGES CXX C)

find_package(Vulkan)

add_subdirectory(External/fmt)
add_subdirectory(External/glfw)
add_subdirectory(External/glm)
add_subdirectory(External/tinyobjloader)
add_subdirectory(External/vk-bootstrap)
add_subdirectory(External/VulkanMemoryAllocator)

set(CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -DNOMINMAX -DUNICODE -D_CRT_SECURE_NO_WARNINGS")

add_subdirectory(Engine)
add_subdirectory(Testbed)