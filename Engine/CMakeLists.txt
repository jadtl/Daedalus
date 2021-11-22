file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(${ENGINE} STATIC ${HEADERS} ${SOURCES})
target_include_directories(${ENGINE} PUBLIC src)