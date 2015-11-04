#ifndef SSQUAD_H
#define SSQUAD_H

class SSQuad
{
public:
    SSQuad();

    void init();
    void draw();

private:
    static const float vertices[8];
    static const float texcoors[8];
    static const unsigned int indices[6];
    unsigned int vvertices;
    unsigned int vindices;
    unsigned int vtexcoors;
    unsigned int vaoHandle;
};

#endif // SSQUAD_H
