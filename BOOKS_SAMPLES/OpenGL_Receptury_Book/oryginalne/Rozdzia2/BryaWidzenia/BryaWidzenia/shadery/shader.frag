#version 330 core

layout (location = 0) out vec4 vFragColor; //wyj�cie shadera fragment�w
//uniform
uniform vec4 color;	//uniform koloru

void main()
{
	//sta�y kolor jako wyj�cie shadera fragment�w
	vFragColor = color;
}