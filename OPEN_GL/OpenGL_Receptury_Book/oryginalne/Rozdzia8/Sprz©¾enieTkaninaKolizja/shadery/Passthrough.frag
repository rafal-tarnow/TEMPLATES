#version 330 core

layout(location=0) smooth out vec4 vFragColor;	//wartoœæ wyjœciowa

//uniform dla koloru
uniform vec4 vColor;

void main()
{ 		
	vFragColor = vColor;  	
}