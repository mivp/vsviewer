#ifndef TEXTURE_H
#define TEXTURE_H

#include "utils.h"
#include "glinclude.h"

#include <string>

using namespace std;

class Texture
{
public:
    Texture(unsigned int _width, unsigned int _height, unsigned int _format=GL_RGBA, unsigned int _globalFormat=GL_RGBA);
    Texture(unsigned int _index, unsigned int _width, unsigned int _height, unsigned int _format=GL_RGBA, unsigned int _globalFormat=GL_RGBA);
    ~Texture();

    void init();
    void bind();
    void update(const unsigned char* buffer, unsigned int w, unsigned int h);
    void resize(unsigned int _width, unsigned int _height);

    void setFilters(unsigned int min, unsigned int mag);

    static void resetUnit(int textureUnitOffset = 0);
    static Texture* newFromNextUnit(unsigned int _width, unsigned int _height, unsigned int _format=GL_RGBA, unsigned int _globalFormat=GL_RGBA);
    static unsigned int unitFromIndex(unsigned int index);

    int getHeight();
    int getWidth();

    // Needs to be public to be accessed by GL calls
    unsigned int gluid;
    unsigned int glunit;
    unsigned int index;

    static bool needsUpdate;

    static const unsigned int LINEAR;
    static const unsigned int NEAREST;
    static const unsigned int MIPMAP;

private:
    static unsigned int unitCount;
    static float borderColor[];
    static float borderColorB[];

    unsigned int height;
    unsigned int width;
    unsigned int minFilter;
    unsigned int magFilter;
    unsigned int format;
    unsigned int globalFormat;
};

#endif // TEXTURE_H
