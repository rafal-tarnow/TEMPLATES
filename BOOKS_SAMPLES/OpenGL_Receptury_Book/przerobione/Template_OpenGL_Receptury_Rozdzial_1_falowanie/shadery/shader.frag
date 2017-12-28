#version 330 core

layout(location=0) out vec4 vFragColor;	//wyjście shadera fragmentów

void main()
{
	//wyprowadzenie białego koloru  vec4(1,1,1,1)
	vFragColor = vec4(1,0,0,1);
}
