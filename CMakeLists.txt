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