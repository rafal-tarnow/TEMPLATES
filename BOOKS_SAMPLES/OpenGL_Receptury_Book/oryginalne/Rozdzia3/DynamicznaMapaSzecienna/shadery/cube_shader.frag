#version 330 core

layout(location = 0) out vec4 vFragColor;	//wyj�ciowy kolor fragmentu

//uniform
uniform vec3 vColor;	//sta�y kolor
void main()
{
	//sta�y kolor jako wyj�cie shadera
	vFragColor = vec4(vColor.xyz,1);
}