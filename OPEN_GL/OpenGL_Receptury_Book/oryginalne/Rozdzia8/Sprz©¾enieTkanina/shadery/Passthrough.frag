#version 330 core

layout(location=0) smooth out vec4 vFragColor;	//warto�� wyj�ciowa

//uniform dla koloru
uniform vec4 vColor;

void main()
{ 		
	vFragColor = vColor;  	
}