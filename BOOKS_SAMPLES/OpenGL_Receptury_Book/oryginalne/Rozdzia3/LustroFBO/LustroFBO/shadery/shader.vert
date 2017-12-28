#version 330 core
  
layout(location=0) in vec3 vVertex; //po�o�enie wierzcho�ka w przestrzeni obiektu

//uniform
uniform mat4 MVP;	//po��czona macierz modelu, widoku i rzutowania

void main()
{ 
	//no�enie po�o�enia wierzcho�ka przez po��czon� macierz modelu, widoku i rzutowania 
        //w celu uzyskania po�o�enia w przestrzeni przyci�cia

	gl_Position = MVP*vec4(vVertex.xyz,1);
}