#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <list>
#include <map>

#include "glinclude.h"
#include "math/vector3.h"
#include "math/matrix4.h"
#include "texture.h"

using namespace std;

class Shader
{
public:
    Shader(string name);
    ~Shader();

    Shader& load(string shader, list<string> attributes, list<string> uniforms);
    Shader& bind();
    Shader& setup();
    Shader& setupLocations(list<string> _attributes, list<string> _uniforms);

    string& getName();

    unsigned int attribute(string name);
    unsigned int uniform(string name);

    bool hasAttribute(string name);
    bool hasUniform(string name);

    void transmitUniform(string name, const Texture* tex);
    void transmitUniform(string name, int i);
    void transmitUniform(string name, float f);
    void transmitUniform(string name, float f1, float f2);
    void transmitUniform(string name, float f1, float f2, float f3);
    void transmitUniform(string name, const Vector3 &vec3);
    void transmitUniform(string name, const Matrix4 &mat4);
    void transmitUniform(string name, bool b);

private:
    string name;
    unsigned int uid;
    const char* vertex;
    const char* fragment;

    map<string, unsigned int> attributes;
    map<string, unsigned int> uniforms;
};

#endif // SHADER_H
