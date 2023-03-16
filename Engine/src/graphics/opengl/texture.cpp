#include "graphics/opengl/texture.h"

#include <glad/glad.h>

namespace ddls::gl {

Texture::Texture():
    internalFormat(GL_RGB), 
    imageFormat(GL_RGB), 
    wrapS(GL_REPEAT), 
    wrapT(GL_REPEAT),
    filterMin(GL_LINEAR),
    filterMax(GL_LINEAR)
{
    glGenTextures(1, &_handle);
}

void Texture::load(ddls::Texture texture)
{
    glBindTexture(GL_TEXTURE_2D, _handle);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        internalFormat, 
        texture.width, 
        texture.height,
        0,
        imageFormat,
        GL_UNSIGNED_BYTE,
        texture.data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMax);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, _handle);
}

} // namespace ddls::gl