#version 330 core 
  
layout(location = 0) in vec2 vVertex; //położenie wierzchołka w przestrzeni obiektu

//wyjście do shadera fragmentów
smooth out vec2 vUV;					
 
void main()
{  
	//przekazanie położenia wierzchołka w przestrzeni obiektu jako współrzędnych tekstury
	//i wyznaczenie położenia wierzchołka w przestrzeni przycięcia
	vUV = vVertex;	
	gl_Position = vec4(vVertex.xy ,0,1);
}