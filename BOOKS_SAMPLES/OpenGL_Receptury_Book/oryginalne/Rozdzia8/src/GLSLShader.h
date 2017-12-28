﻿//Prosta klasa do kompilowania shaderów GLSL
//Autor Movania Muhammad Mobeen
#pragma once
#include <GL/glew.h>
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
	void LoadFromFile(GLenum whichShader, const string& filename, const string& shaderCodeToAddInHeaderBeforeMain);

	void CreateAndLinkProgram();
	void Use();
	void UnUse();
	void AddAttribute(const string& attribute);
	void AddUniform(const string& uniform);
	GLuint GetProgram() const;
	//Indekser zwracający lokalizację atrybutu (uniformu)
	GLuint operator[](const string& attribute);
	GLuint operator()(const string& uniform);
	void DeleteShaderProgram(); 

private:
	enum ShaderType {VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER};
	GLuint	_program;
	int _totalShaders;
	GLuint _shaders[3];//0->vertexshader, 1->fragmentshader, 2->geometryshader
	map<string,GLuint> _attributeList;
	map<string,GLuint> _uniformLocationList;
};	
