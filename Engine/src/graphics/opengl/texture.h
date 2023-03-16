#pragma once

#include "core/defines.h"
#include "core/types.h"
#include "core/resources.h"

namespace ddls::gl {

class DDLS_API Texture
{
public:
    /**
     * @brief The handle to the texture
     * 
     */
    u32 handle() const { return _handle; }

    /**
     * @brief Configurable variables
     * 
     */
    i32 internalFormat;
    u32 imageFormat;
    i32 wrapS;
    i32 wrapT;
    i32 filterMin;
    i32 filterMax;

    /**
     * @brief Generates a texture and fills the default variables
     * 
     */
    Texture();

    /**
     * @brief Load an engine's texture
     * 
     * @param texture The texture to load
     */
    void load(ddls::Texture texture);

    /**
     * @brief Binds texture as active texture object
     * 
     */
    void bind() const;

private:
    u32 _handle;
};

} // namespace ddls::gl