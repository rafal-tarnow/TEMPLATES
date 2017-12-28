#version 330 core
  
layout(location=0) in vec3 vVertex;		//po�o�enie wierzcho�ka w przestrzeni obiektu
layout(location=1) in vec3 vNormal;		//wektor normalny w przestrzeni obiektu
 
//uniform
uniform mat4 MVP;	//po��czona macierz modelu, widoku i rzutowania

//wyj�cie shadera
smooth out vec3 position;	//po�o�enie wierzcho�ka w przestrzeni obiekt
smooth out vec3 normal;		//wektor normalny w przestrzeni obiektu

void main()
{ 	
	//wyprowadzenie po�o�enia i wektora normalnego w przestrzeni obiektu
	position = vVertex;
	normal = vNormal;

	//mno�enie po�o�enia w przestrzeni obiektu przez po��czon� macierz MVP w celu uzyskania po�o�enia w przestrzeni przyci�cia 
    gl_Position = MVP*vec4(vVertex,1); 
}
 