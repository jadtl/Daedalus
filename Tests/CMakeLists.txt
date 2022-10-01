add_executable(Endian src/endian.cpp)
include_directories(src/)
target_link_libraries(Endian Daedalus::Engine)