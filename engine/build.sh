#!/bin/bash
# Build script for engine
set echo on

mkdir -p ../bin

# Get a list of all the .c files.
cFilenames=$(find . -type f -name "*.c")

echo "Files:" $cFilenames

assembly="daedalus"
compilerFlags="-g -shared -fdeclspec -fPIC"
# -fms-extensions 
# -Wall -Werror
includeFlags="-Isrc -I/usr/include/vulkan"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L/usr/lib -L/usr/X11R6/lib"
defines="-D_DEBUG -D_EXPORT"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags