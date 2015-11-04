#include "texture.h"
#include <iostream>
using namespace std;

const unsigned int Texture::LINEAR = GL_LINEAR;
const unsigned int Texture::NEAREST = GL_NEAREST;
const unsigned int Texture::MIPMAP = GL_LINEAR_MIPMAP_LINEAR;

Texture::Texture(unsigned int _width, unsigned int _height, unsigned int _format, unsigned int _globalFormat)
{
    width = _width;
    height = _height;
    format = _format;
    gluid = 0;
    glunit = 0;
    index = 0;
    minFilter = GL_LINEAR;
    magFilter = GL_LINEAR;
    globalFormat = _globalFormat;
}

Texture::Texture(unsigned int _index, unsigned int _width, unsigned int _height, unsigned int _format, unsigned int _globalFormat)
{
    height = _height;
    width = _width;
    format = GL_RGB8;
    index = _index;
    glunit = unitFromIndex(_index);
    gluid = 0;
    minFilter = GL_LINEAR;
    magFilter = GL_LINEAR;
    globalFormat = _globalFormat;
}

Texture::~Texture()
{
    glDeleteTextures(1, &gluid);
}

void Texture::setFilters(unsigned int min, unsigned int mag)
{
    minFilter = min;
    magFilter = mag;
}

void Texture::init()
{
    glActiveTexture(glunit);
    glGenTextures(1, &gluid);
    glBindTexture(GL_TEXTURE_2D, gluid);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, globalFormat, GL_UNSIGNED_BYTE, NULL); //GL_UNSIGNED_INT_8_8_8_8_REV
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP_TO_BORDER GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture::bind()
{
    glActiveTexture(glunit);
    glBindTexture(GL_TEXTURE_2D, gluid);
}

void Texture::update(const unsigned char* buffer, unsigned int w, unsigned int h)
{
    if(w != width || h != height)
    {
        width = w;
        height = h;
        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, globalFormat, GL_UNSIGNED_BYTE, buffer);
    }
    else
    {
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, globalFormat, GL_UNSIGNED_BYTE, buffer);
    }
}

void Texture::resize(unsigned int _width, unsigned int _height)
{
    width = _width;
    height = _height;

    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, globalFormat, GL_UNSIGNED_BYTE, NULL);
}

int Texture::getHeight()
{
    return height;
}

int Texture::getWidth()
{
    return width;
}


/*!
 * @static
 */

bool Texture::needsUpdate = false;

unsigned int Texture::unitCount = 0;

float Texture::borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
float Texture::borderColorB[] = {0.0f, 0.0f, 0.0f, 0.0f};

void Texture::resetUnit(int textureUnitOffset)
{
    unitCount = textureUnitOffset;
}

Texture* Texture::newFromNextUnit(unsigned int _width, unsigned int _height, unsigned int _format, unsigned int _globalFormat)
{
    return new Texture(unitCount++, _width, _height, _format, _globalFormat);
}

unsigned int Texture::unitFromIndex(unsigned int index)
{
    switch(index)
    {
        case 1: return GL_TEXTURE1;
        case 2: return GL_TEXTURE2;
        case 3: return GL_TEXTURE3;
        case 4: return GL_TEXTURE4;
        case 5: return GL_TEXTURE5;
        case 6: return GL_TEXTURE6;
        case 7: return GL_TEXTURE7;
        case 8: return GL_TEXTURE8;
        case 9: return GL_TEXTURE9;
        default: return GL_TEXTURE0;
    }
}
