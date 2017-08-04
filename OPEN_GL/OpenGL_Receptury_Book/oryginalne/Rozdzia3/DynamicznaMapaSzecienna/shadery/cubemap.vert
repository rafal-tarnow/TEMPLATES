#version 330 core
  
layout(location=0) in vec3 vVertex;		//po³o¿enie wierzcho³ka w przestrzeni obiektu
layout(location=1) in vec3 vNormal;		//wektor normalny w przestrzeni obiektu
 
//uniform
uniform mat4 MVP;	//po³¹czona macierz modelu, widoku i rzutowania

//wyjœcie shadera
smooth out vec3 position;	//po³o¿enie wierzcho³ka w przestrzeni obiekt
smooth out vec3 normal;		//wektor normalny w przestrzeni obiektu

void main()
{ 	
	//wyprowadzenie po³o¿enia i wektora normalnego w przestrzeni obiektu
	position = vVertex;
	normal = vNormal;

	//mno¿enie po³o¿enia w przestrzeni obiektu przez po³¹czon¹ macierz MVP w celu uzyskania po³o¿enia w przestrzeni przyciêcia 
    gl_Position = MVP*vec4(vVertex,1); 
}
 