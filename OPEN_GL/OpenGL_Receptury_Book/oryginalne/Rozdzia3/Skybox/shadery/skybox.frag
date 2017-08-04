#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyj�cie shadera fragment�w
	
//uniform
uniform samplerCube cubeMap;	//sampler tekstury sze�ciennej

//dane z shadera wierzcho�k�w
smooth in vec3 uv;	//interpolowane wsp�rz�dne 3D dla tekstury

void main()
{
	//zwracanie koloru pobranego z tekstury
	vFragColor = texture(cubeMap, uv);
}