# Daedalus

Daedalus is a work-in-progress graphics engine written in C++ using the Vulkan graphics API.

## Dependencies
- [Vulkan](https://www.vulkan.org/) - Graphics API

- [VkBootstrap](https://github.com/charles-lunarg/vk-bootstrap) - Vulkan boilerplate

- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan memory allocation

- [GLFW](https://github.com/glfw/glfw) - Window and input

- [GLM](https://github.com/g-truc/glm) - Linear algebra

- [Boost](https://www.boost.org/) - Filesystem

- [TinyOBJLoader](https://github.com/tinyobjloader/tinyobjloader) - OBJ models loading

## Usage
1. [Install](https://www.lunarg.com/vulkan-sdk/) the Vulkan SDK

2. Clone the repository with its submodules

`git clone --recurse-submodules https://github.com/JadTala/Daedalus.git`

3. Use the CMake files to compile either the library or the provided testbed

## License
[MIT](https://choosealicense.com/licenses/mit/)