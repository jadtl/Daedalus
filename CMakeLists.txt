cmake_minimum_required(VERSION 3.16)

set(ENGINE Daedalus)
set(TESTBED Testbed)

project(ENGINE)

find_package(Vulkan)

add_subdirectory(External/boost)
add_subdirectory(External/glm)
add_subdirectory(External/glfw)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNOMINMAX -DUNICODE")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -DNOMINMAX -DUNICODE")

add_subdirectory(Engine)
add_subdirectory(Testbed)

target_link_libraries(${ENGINE} Vulkan::Vulkan glm Boost::filesystem glfw)