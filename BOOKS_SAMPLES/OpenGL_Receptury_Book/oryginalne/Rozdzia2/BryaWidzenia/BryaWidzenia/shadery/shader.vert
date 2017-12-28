#version 330 core
  
layout (location=0) in vec3 vVertex; //po³o¿enie wierzcho³ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;  //po³¹czona macierz modelu, widoku i rzutowania

void main()
{  
	//wyznaczanie po³o¿enia wierzcho³ka w przestrzeni przyciêcia 	
	gl_Position = MVP*vec4(vVertex.xyz,1);
}