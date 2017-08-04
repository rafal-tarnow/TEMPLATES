#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyjœcie shadera fragmentów
	
//uniform
uniform samplerCube cubeMap;	//sampler tekstury szeœciennej

//dane z shadera wierzcho³ków
smooth in vec3 uv;	//interpolowane wspó³rzêdne 3D dla tekstury

void main()
{
	//zwracanie koloru pobranego z tekstury
	vFragColor = texture(cubeMap, uv);
}