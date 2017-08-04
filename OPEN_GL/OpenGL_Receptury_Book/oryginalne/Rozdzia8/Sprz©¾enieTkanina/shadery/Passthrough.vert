#version 330 core
precision highp float;

layout (location=0) in vec4 position_mass;	//położenie i masa
uniform mat4 MVP;							//połączona macierz modelu, widoku i rzutowania

void main() 
{  
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia
	gl_Position = MVP*vec4(position_mass.xyz, 1.0);		
}