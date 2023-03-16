#pragma once

#include "core/defines.h"
#include "core/types.h"

#include <map>
#include <filesystem>

namespace ddls {

/**
 * @brief The engine's representation of a texture
 * 
 */
struct DDLS_API Texture
{
    u16 width;
    u16 height;
    u16 channels;
    unsigned char* data;
};

/**
 * @brief A resource management class
 *
 */
class DDLS_API Resources
{
public:
    ~Resources();

    /**
     * @brief Returns the single instance of the class
     *
     */
    static Resources& Manager()
    {
        static Resources manager;

        return manager;
    }
    Resources(Resources const&)       = delete;
    void operator=(Resources const&)  = delete;

    std::filesystem::path getPath(const char* filePath);

    /**
     * @brief Gets the requested file, allocating its contents if needed
     *
     */
    const char* getFile(const char* filePath);

    /**
     * @brief Gets the requested texture, allocating its contents if needed
     *
     */
    const Texture getTexture(const char* texturePath);

    /**
     * @brief Frees the given resource
     *
     */
    void free(const char* filePath);

private:
    Resources() = default;
    std::map<const char*, char*> _files;
    std::map<const char*, Texture> _textures;
    static std::filesystem::path cwd();
};

} // namespace ddls
