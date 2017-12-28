#version 330 core
  
layout(location=0) in vec3 vVertex;  //położenie wierzchołka w przestrzeni obiektu

uniform mat4 M[4];	//macierz modelu dla każdej instancji

void main()
{    
	//ustawienie położenia w przestrzeni obiektu dla każdego wierzchołka instancji
	gl_Position =  M[gl_InstanceID]*vec4(vVertex, 1);		
}
