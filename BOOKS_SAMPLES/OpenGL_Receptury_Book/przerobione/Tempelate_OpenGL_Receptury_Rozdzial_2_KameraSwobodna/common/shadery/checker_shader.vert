#version 330 core
  
layout(location = 0) in vec3 vVertex;	//położenie w przestrzeni obiektu

//uniform
uniform mat4 MVP;	//połączona macierz modelu, widoku i rzutowania

//wyjście shadera wierzchołków
smooth out vec2 vUV;	//współrzędne tekstury 2D

void main()
{  
	//mnożenie położenia w przestrzeni obiektu przez macierz MVP w celu uzyskania położenia w przestrzeni przycięcia 
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//współrzędne x i z wierzchołka jako współrzędne tekstury 2D
	vUV =   (vVertex.xz); 
}
