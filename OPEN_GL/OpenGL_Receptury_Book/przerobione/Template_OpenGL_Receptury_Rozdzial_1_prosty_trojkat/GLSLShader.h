#pragma once

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

//#include <GL/glew.h>
#include <map>
#include <string>

using namespace std;

class GLSLShader
{
public:
    GLSLShader(void);
    ~GLSLShader(void);
    void LoadFromString(GLenum whichShader, const string& source);
    void LoadFromFile(GLenum whichShader, const string& filename);
    void CreateAndLinkProgram();
    void Use();
    void UnUse();
    void AddAttribute(const string& attribute);
    void AddUniform(const string& uniform);

    //indekser zwracajšcy lokalizację atrybutu (uniformu)
    GLuint operator[](const string& attribute);
    GLuint operator()(const string& uniform);
    void DeleteShaderProgram();

private:
    enum ShaderType {VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER};
    GLuint	_program;
    int _totalShaders;
    GLuint _shaders[3];//0->shader wierzchołków, 1->shader fragmentów, 2->shader geometrii
    map<string,GLuint> _attributeList;
    map<string,GLuint> _uniformLocationList;
};
