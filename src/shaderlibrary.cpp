#include "shaderlibrary.h"
#include "utils.h"

map<string, Shader*> ShaderLibrary::shaders;

Shader* ShaderLibrary::getShader(string name)
{
    if(shaders.find(name) != shaders.end())
        return shaders.at(name);

    printf("Error: Shader doesn't exist in ShaderLibrary");
    //QMessageBox::critical(Utils::WINDOW, "Error", "Shader '"+name+"' doesn't exist in ShaderLibrary.");
    exit(-1);
}

Shader* ShaderLibrary::addShader(string name, string shaderPrefix, list<string> attributes, list<string> uniforms)
{
    if(shaders.find(name) != shaders.end())
    {
        //QMessageBox::warning(Utils::WINDOW, "Warning", "Duplicate shader '"+name+"' in ShaderLibrary.");
        return shaders.at(name);
    }

    Shader* shader = new Shader(name);
    shader->load(shaderPrefix, attributes, uniforms);

    shaders.insert(pair<string, Shader*> (name, shader));

    return shader;
}

void ShaderLibrary::printShaders()
{
    for(map<string, Shader*>::iterator it = shaders.begin(); it != shaders.end(); ++it)
        printf("%s", it->first.c_str()); 
}

void ShaderLibrary::clear()
{
    //foreach(Shader* shader, shaders)
    //    delete shader;
    map<string, Shader*>::iterator it;
    for (it=shaders.begin(); it!=shaders.end(); ++it)
        delete it->second;
    shaders.clear();
}
