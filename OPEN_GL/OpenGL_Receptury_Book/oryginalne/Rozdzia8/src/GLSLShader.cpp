//Prosta klasa do kompilowania shaderów GLSL
//Autor Movania Muhammad Mobeen
//ostatnia modyfikacja:  02 lutego 2011

#include "GLSLShader.h"
#include <iostream>


GLSLShader::GLSLShader(void)
{
	_totalShaders=0;
	_shaders[VERTEX_SHADER]=0;
	_shaders[FRAGMENT_SHADER]=0;
	_shaders[GEOMETRY_SHADER]=0;
	_attributeList.clear();
	_uniformLocationList.clear();
}

GLSLShader::~GLSLShader(void)
{
	_attributeList.clear();	
	_uniformLocationList.clear();	
}
void GLSLShader::DeleteShaderProgram() {	
	glDeleteProgram(_program);
}

void GLSLShader::LoadFromString(GLenum type, const string& source) {	
	GLuint shader = glCreateShader (type);

	const char * ptmp = source.c_str();
	glShaderSource (shader, 1, &ptmp, NULL);
	
	//sprawdź, czy shader wczytuje się prawidłowo
	GLint status;
	glCompileShader (shader);
	glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;		
		glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog= new GLchar[infoLogLength];
		glGetShaderInfoLog (shader, infoLogLength, NULL, infoLog);
		cerr<<"Compile log: "<<infoLog<<endl;
		delete [] infoLog;
	}
	_shaders[_totalShaders++]=shader;
}


void GLSLShader::CreateAndLinkProgram() {
	_program = glCreateProgram ();
	if (_shaders[VERTEX_SHADER] != 0) {
		glAttachShader (_program, _shaders[VERTEX_SHADER]);
	}
	if (_shaders[FRAGMENT_SHADER] != 0) {
		glAttachShader (_program, _shaders[FRAGMENT_SHADER]);
	}
	if (_shaders[GEOMETRY_SHADER] != 0) {
		glAttachShader (_program, _shaders[GEOMETRY_SHADER]);
	}
	
	//skonsoliduj i sprawdź, czy konsolidacja programu przebiegła prawidłowo 
	GLint status;
	glLinkProgram (_program);
	glGetProgramiv (_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		
		glGetProgramiv (_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog= new GLchar[infoLogLength];
		glGetProgramInfoLog (_program, infoLogLength, NULL, infoLog);
		cerr<<"Link log: "<<infoLog<<endl;
		delete [] infoLog;
	}

	glDeleteShader(_shaders[VERTEX_SHADER]);
	glDeleteShader(_shaders[FRAGMENT_SHADER]);
	glDeleteShader(_shaders[GEOMETRY_SHADER]);
}

void GLSLShader::Use() {
	glUseProgram(_program);
}

void GLSLShader::UnUse() {
	glUseProgram(0);
}

void GLSLShader::AddAttribute(const string& attribute) {
	_attributeList[attribute]= glGetAttribLocation(_program, attribute.c_str());	
}

//Indekser zwracający lokalizację atrybutu
GLuint GLSLShader::operator [](const string& attribute) {
	return _attributeList[attribute];
}

void GLSLShader::AddUniform(const string& uniform) {
	_uniformLocationList[uniform] = glGetUniformLocation(_program, uniform.c_str());
}

GLuint GLSLShader::operator()(const string& uniform){
	return _uniformLocationList[uniform];
}
GLuint GLSLShader::GetProgram() const {
	return _program;
}
#include <fstream>
void GLSLShader::LoadFromFile(GLenum whichShader, const string& filename){
	ifstream fp;
	fp.open(filename.c_str(), ios_base::in);
	if(fp) {		 
		string buffer(std::istreambuf_iterator<char>(fp), (std::istreambuf_iterator<char>()));
		LoadFromString(whichShader, buffer);		
	} else {
		cerr<<"Error loading shader: "<<filename<<endl;
	}
}

void GLSLShader::LoadFromFile(GLenum whichShader, const string& filename, const string& shaderCodeToAddInHeaderBeforeMain) {
	ifstream fp;
	fp.open(filename.c_str(), ios_base::in);
	if(fp) {		  
		string buffer(std::istreambuf_iterator<char>(fp), (std::istreambuf_iterator<char>()));
		string temp="";
		//znajdź początek funkcji main()
		int index = buffer.find("void");
		if(index!=string::npos) {
			//funkcja main() znaleziona
			string header = buffer.substr(0, index-1);
			string footer = buffer.substr(index, buffer.length());
			temp.append(header);
			temp.append(shaderCodeToAddInHeaderBeforeMain);
			temp.append(footer);
		}
		//cout<<temp<<endl; 
		LoadFromString(whichShader, temp);		
	} else {
		cerr<<"Blad wczytywania shadera: "<<filename<<endl;
	}
}
