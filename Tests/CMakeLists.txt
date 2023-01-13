include_directories(src/)
link_libraries(Daedalus::Engine)

add_executable(Endian src/endian.cpp)
if (WIN32)
    target_compile_definitions(Endian
        PRIVATE
        DDLS_EXPORT)
endif()
add_executable(Allocator src/allocator.cpp)
if (WIN32)
    target_compile_definitions(Allocator
        PRIVATE
        DDLS_EXPORT)
endif()