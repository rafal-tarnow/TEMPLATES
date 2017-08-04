#version 330 core

layout (location = 0) out vec4 vFragColor; //wyjœcie shadera fragmentów
//uniform
uniform vec4 color;	//uniform koloru

void main()
{
	//sta³y kolor jako wyjœcie shadera fragmentów
	vFragColor = color;
}