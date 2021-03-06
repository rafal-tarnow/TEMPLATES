#version 330 core
  
layout(location=0) in vec3 vVertex;		//położenie wierzchołka w przestrzeni obiektu
layout(location=1) in vec3 vNormal;		//wektor normalny w przestrzeni obiektu
 
//uniform
uniform mat4 MVP;	//połączona macierz modelu, widoku i rzutowania

//wyjście shadera
smooth out vec3 position;	//położenie wierzchołka w przestrzeni obiekt
smooth out vec3 normal;		//wektor normalny w przestrzeni obiektu

void main()
{ 	
	//wyprowadzenie położenia i wektora normalnego w przestrzeni obiektu
	position = vVertex;
	normal = vNormal;

	//mnożenie położenia w przestrzeni obiektu przez połączoną macierz MVP w celu uzyskania położenia w przestrzeni przycięcia 
    gl_Position = MVP*vec4(vVertex,1); 
}
 