# Daedalus

Daedalus is a work-in-progress graphics engine written in C++ using the Vulkan graphics API.

## Dependencies
- [Vulkan](https://www.vulkan.org/) - Graphics API

- [vk-bootstrap](https://github.com/charles-lunarg/vk-bootstrap) - Vulkan boilerplate

- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan memory allocation

- [GLFW](https://github.com/glfw/glfw) - Window and input

- [GLM](https://github.com/g-truc/glm) - Linear algebra

- [{fmt}](https://github.com/fmtlib/fmt) - String formatting

- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) - OBJ models loading

## Usage
1. [Install](https://www.lunarg.com/vulkan-sdk/) the Vulkan SDK

2. Clone the repository with its submodules

`git clone --recurse-submodules https://github.com/JadTala/Daedalus.git`

3. Use the CMake files to compile the library and the provided testbed

## License
[MIT](https://choosealicense.com/licenses/mit/)