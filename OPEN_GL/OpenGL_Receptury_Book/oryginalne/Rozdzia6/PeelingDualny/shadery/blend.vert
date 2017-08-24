#version 330 core 
  
layout(location = 0) in vec2 vVertex; //położenie wierzchołka w przestrzeni obiektu
 
void main()
{  
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia na podstawie jego położenia w przestrzeni obiektu
	gl_Position = vec4(vVertex.xy*2 - 1.0,0,1);
}