#ifndef SHADERLIBRARY_H
#define SHADERLIBRARY_H

#include "shader.h"
#include <map>
#include <string>

using namespace std;

class ShaderLibrary
{
public:
    static Shader* getShader(string name);
    static Shader* addShader(string name, string shaderPrefix, list<string> attributes, list<string> uniforms);
    static void printShaders();
    static void clear();

private:
    static map<string, Shader*> shaders;
};

#endif // SHADERLIBRARY_H
