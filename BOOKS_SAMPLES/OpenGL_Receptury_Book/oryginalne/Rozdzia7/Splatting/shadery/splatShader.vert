#version 330 core
  
layout(location = 0) in vec3 vVertex;	//położenie wierzchołka w przestrzeni obiektu
layout(location = 1) in vec3 vNormal;	//normalna wierzchołka w przestrzeni obiektu
   
//uniformy
uniform mat4 MV;			//macierz modelu i widoku
uniform mat3 N;				//macierz normalna
uniform mat4 P;				//macierz rzutowania		
uniform float splatSize;	//rozmiar placka

smooth out vec3 outNormal;	//normalna w przestrzeni oka

void main()
{    
	//wyznaczanie położenia wierzchołka w przestrzeni oka
	vec4 eyeSpaceVertex = MV*vec4(vVertex,1);
	
	//wyznaczanie rozmiaru placka w zależności od współrzędnej z wierzchołka w przestrzeni oka
	gl_PointSize = 2*splatSize/-eyeSpaceVertex.z; 
	
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia
	gl_Position = P * eyeSpaceVertex; 

	//wyznaczanie normalnej w przestrzeni oka
    outNormal = N*vNormal;
}