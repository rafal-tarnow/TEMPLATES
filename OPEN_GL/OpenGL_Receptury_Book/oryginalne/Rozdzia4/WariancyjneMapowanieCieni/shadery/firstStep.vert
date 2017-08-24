#version 330 core
  
layout (location = 0 ) in vec3 vVertex; //po�o�enie wierzcho�ka w przestrzeni obiektu

uniform mat4 MVP;	//po��czona macierz modelu, widoku i rzutowania

//wyj�cie shadera
smooth out vec4 clipSpacePos;	//po�o�enie w przestrzeni przyci�cia
void main()
{ 	 
	//mno�enie po�o�enia w przestrzeni obiektu przez po��czon� macierz MVP 
	//w celu uzyskania po�o�enia w przestrzeni przyci�cia
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//wyprowadzenie po�o�enia w przestrzeni przyci�cia
	clipSpacePos = gl_Position;
}