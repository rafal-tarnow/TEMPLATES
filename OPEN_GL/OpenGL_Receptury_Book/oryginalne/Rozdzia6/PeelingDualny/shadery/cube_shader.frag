#version 330 core

layout(location = 0) out vec4 vFragColor; //wyjściowy kolor fragmentu

uniform vec4 vColor;	//uniform koloru

void main()
{
	//przypisanie fragmentowi koloru zapisanego w uniformie vColor
   vFragColor = vColor;
}