include_directories(src/)
link_libraries(Daedalus::Engine)

add_executable(Endian src/endian.cpp)
add_executable(Allocator src/allocator.cpp)