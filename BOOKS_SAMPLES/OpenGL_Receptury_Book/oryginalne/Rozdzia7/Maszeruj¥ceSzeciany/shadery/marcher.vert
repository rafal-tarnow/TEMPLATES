#version 330 core
  
layout(location = 0) in vec3 vVertex;	//położenie wierzchołka w przestrzeni obiektu
layout(location = 1) in vec3 vNormal;	//normalna wierzchołka w przestrzeni obiektu

//połączona macierz modelu, widoku i rzutowania 
uniform mat4 MVP;  
 
//normalna w przestrzeni obiektu
smooth out vec3 outNormal;

void main()
{  
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia
	gl_Position = MVP*vec4(vVertex.xyz,1);
	//normalna w przestrzeni obiektu
    outNormal = vNormal;
}