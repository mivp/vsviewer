#include "ssquad.h"
#include "glinclude.h"

SSQuad::SSQuad()
{
    vvertices = 0;
    vtexcoors = 0;
    vindices = 0;
}

const float SSQuad::vertices[8] = {
    -1.0f,   -1.0f,
     1.0f,   -1.0f,
    -1.0f,    1.0f,
     1.0f,    1.0f
};

const float SSQuad::texcoors[8] = {
    0.0f,   1.0f,
    1.0f,   1.0f,
    0.0f,   0.0f,
    1.0f,   0.0f
};

const unsigned int SSQuad::indices[6] = {0, 1, 2, 1, 3, 2};

void SSQuad::init()
{
    glGenBuffers(1,&vvertices);
    glBindBuffer(GL_ARRAY_BUFFER,vvertices);
    glBufferData(GL_ARRAY_BUFFER,8*sizeof(float),vertices,GL_STATIC_DRAW);

    glGenBuffers(1,&vindices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vindices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,6*sizeof(int),indices,GL_STATIC_DRAW);

    glGenBuffers(1,&vtexcoors);
    glBindBuffer(GL_ARRAY_BUFFER,vtexcoors);
    glBufferData(GL_ARRAY_BUFFER,8*sizeof(float),texcoors,GL_STATIC_DRAW);
  
    // Create and set-up the vertex array object
    glGenVertexArrays( 1, &vaoHandle );
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);  // position
    glBindBuffer(GL_ARRAY_BUFFER, vvertices);
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );

    glEnableVertexAttribArray(1);  // tex coor
    glBindBuffer(GL_ARRAY_BUFFER, vtexcoors);
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );
}

void SSQuad::draw()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vindices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
