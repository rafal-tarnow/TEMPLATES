#version 330 core
  
layout (location=0) in vec3 vVertex; //po�o�enie wierzcho�ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;  //po��czona macierz modelu, widoku i rzutowania

void main()
{  
	//wyznaczanie po�o�enia wierzcho�ka w przestrzeni przyci�cia 	
	gl_Position = MVP*vec4(vVertex.xyz,1);
}