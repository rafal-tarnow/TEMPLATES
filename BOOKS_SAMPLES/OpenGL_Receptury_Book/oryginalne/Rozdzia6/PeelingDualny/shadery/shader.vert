#version 330 core
  
layout(location = 0) in vec3 vVertex; //położenie wierzchołka w przestrzeni obiektu

//uniform
uniform mat4 MVP;  //połączona macierz modelu, widoku i rzutowania

void main()
{  
	//wyznaczanie położenia wierzchołka w przestrzeni przycięcia
	gl_Position = MVP*vec4(vVertex.xyz,1);
}