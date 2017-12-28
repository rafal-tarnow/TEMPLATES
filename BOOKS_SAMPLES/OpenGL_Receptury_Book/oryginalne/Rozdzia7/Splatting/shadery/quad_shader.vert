#version 330 core
  
layout(location=0) in vec2 vVertex;	//położenie wierzchołka w przestrzeni obiektu		

smooth out vec2 vUV;	//wyprowadzenie współrzędnych tekstury 
						//dla shadera fragmentów

void main()
{ 	 
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia na podstawie jego położenia w przestrzeni obiektu
	gl_Position = vec4(vVertex.xy*2.0-1.0,0,1); 

	//przekazanie położenia wierzchołka w przestrzeni obiektu jako współrzędnych tekstury
	vUV = vVertex; 
}