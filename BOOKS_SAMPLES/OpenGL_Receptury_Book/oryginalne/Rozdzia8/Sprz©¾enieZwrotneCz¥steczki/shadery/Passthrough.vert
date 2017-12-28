#version 330 core
precision highp float;

layout (location=0) in vec4 position;	//położenie wierzchołka w przestrzeni obiektu 
uniform mat4 MVP;						//połączona macierz modelu, widoku i rzutowania

void main() 
{  
	//wyznacz położenie wierzchołka w przestrzeni przycięcia
	gl_Position = MVP*vec4(position.xyz, 1.0);		
}